/*
Author:	DarkAngel

TODO:
	- notify booting status
	- keep server-nodes status
	- scan devices
*/
#include "Arduino.h"
#include "Houz.h"
#include <QueueArray.h>
#include <RF24.h>
#include <printf.h>
#include <avr/wdt.h> //watchdog

//radio setup
#define rfChannel		0x5B   
#define rf_server_tx	0xA0
#define rf_server_rx	0xB0
#define rf_office_tx	0xA1
#define rf_office_rx	0xB1
#define rf_suite_tx		0xA2
#define rf_suite_rx		0xB2
#define rf_living_tx	0xA3
#define rf_living_rx	0xB3

#define rf_led_low		0x33
#define rf_led_idle		0x66
#define rf_led_high		0x99
#define rf_led_max		0xFF

#define anim_blink		0x1
#define anim_void			0x0

HouzDevicesCodec* codec;

bool server_online = false;
Houz::Houz(byte NodeId, RF24 &_radio, byte _statusLed, Stream &serial) {
	init(NodeId, _radio, _statusLed, serial);
};

Houz::Houz(byte NodeId, RF24 &_radio, byte _statusLed, Stream &serial, u8 _dataPin, u8 _latchPin, u8 _clockPin) {
	//74HC595 shift register setup
	ioReady = true;
	dataPin = _dataPin;
	latchPin = _latchPin;
	clockPin = _clockPin;
	ioStatus = 0;
	pinMode(latchPin, OUTPUT);
	pinMode(clockPin, OUTPUT);
	pinMode(dataPin, OUTPUT);
	digitalWrite(dataPin, 0);
	digitalWrite(clockPin, 0);
	digitalWrite(latchPin, 0);
	shiftOut(dataPin, clockPin, MSBFIRST, 0);
	shiftOut(dataPin, clockPin, MSBFIRST, 0);
	digitalWrite(latchPin, 1);

	init(NodeId, _radio, _statusLed, serial);
};

void Houz::init(byte NodeId, RF24 &_radio, byte _statusLed, Stream &serial) {
	console = &serial;
	radio = &_radio;
	node_id = NodeId;
	statusLed = _statusLed;
	pinMode(statusLed, OUTPUT);
	analogWrite(statusLed, rf_led_low);
}

void Houz::setup() {
	wdt_enable(WDTO_8S);
	radioSetup();
	
	if (node_id == server_node) return;

	console->print(F("-- "));
	console->print(node_name());
	console->println(F(" ready --\n"));

	//notify server
	radioSend(CMD_STATUS, node_id, 0xF0F0);
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Commands

bool Houz::hasData() {
	wdt_reset(); //watchdog 

	//listeners
	if (radioRead()) return true;
	if (serialRead()) return true;

	//processes
	statusLedRender();
	radioWrite();
	return !commandsQueue.isEmpty();
};

void Houz::pushData(u8 deviceCmd, u8 deviceId, u32 devicePayload){
	deviceData dev;
	dev.cmd = deviceCmd;
	dev.id = deviceId;
	dev.payload = devicePayload;
	pushData(dev);
}

void Houz::pushData(deviceData device) {
	commandsQueue.enqueue(device);
};

deviceData Houz::getData() {
	return commandsQueue.dequeue();
};

void Houz::printToHost(byte result, byte node, u32 message){
	console->print(F("["));
	console->print(result);
	console->print(F("N"));
	console->print(node, HEX);
	console->print(message, HEX);
	console->println(F("]"));
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Helpers 
void Houz::statusLedBlink() {
	statusLedAnim.id = anim_blink;
	statusLedAnim.on = true;
	statusLedLevel = 0x66;
};
void Houz::statusLedVoid() {
	statusLedAnim.id = anim_void;
	statusLedAnim.on = true;
	statusLedLevel = 0x66;
};
int statusLedLevel = 0;
void Houz::statusLedRender() {
	if (!statusLedAnim.on || statusLedAnim.nextStep > millis()) return;
	statusLedAnim.nextStep = millis() + 50;

	//step
	switch (statusLedAnim.id)
	{
		case anim_blink:
			statusLedLevel+=0xF;
			statusLedAnim.on = statusLedLevel<0xFF;
			break;
		case anim_void:
			statusLedLevel-=0xF;
			statusLedAnim.on = statusLedLevel>0;
			break;
	}
	analogWrite(statusLed, statusLedAnim.on?statusLedLevel:(radio_status?rf_led_idle:rf_led_low));
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Radio Stuff 
void Houz::radioSetup()
{
	radio_status = 0;
	pinMode(statusLed, OUTPUT);
	analogWrite(statusLed, rf_led_low);

	//setup rf24 module
	printf_begin();
	radio->begin();
	radio->setPALevel(RF24_PA_MAX); //RF24_PA_HIGH | RF24_PA_LOW
	radio->setDataRate(RF24_1MBPS);
	radio->enableDynamicAck();
	radio->setCRCLength(RF24_CRC_8);
	radio->setChannel(rfChannel);
	radio->setRetries(15, 15);

	//determine device pipes
	setPipes();

	//radio status
	radio_status = (rfChannel == radio->getChannel()); //test if radio is enabled

	if (node_id == server_node) {
		console->println(radio_status ? F("[online]") : F("[offline]"));
	}
	else {
		console->print(F("device: "));
		console->print(node_name());
		console->print(F(" ["));
		console->print(node_id);
		console->println(F("]"));
		console->print(F("status: "));
		console->println(radio_status ? F("online") : F("offline"));
		radio->printDetails();
	}
	if (radio_status) 
		radio->startListening();
	statusLedBlink();
};

void Houz::setPipes() {
	switch (node_id)
	{
	case server_node: 
		radio->openWritingPipe(rf_server_tx);
		radio->openReadingPipe(1, rf_office_tx);
		radio->openReadingPipe(2, rf_suite_tx);
		radio->openReadingPipe(3, rf_living_tx);
		break;

	case office_node:
		radio->openWritingPipe(rf_office_tx);
		radio->openReadingPipe(1, rf_office_rx);
		break;

	case suite_node: 
		radio->openWritingPipe(rf_suite_tx);
		radio->openReadingPipe(1, rf_suite_rx);
		break;

	case living_node: 
		radio->openWritingPipe(rf_living_tx);
		radio->openReadingPipe(1, rf_living_rx);
		break;

	default:
		break;
	}
};

String Houz::node_name() {
	switch (node_id){
	case server_node:	return F("server_node"); 
	case office_node:	return F("office_node");
	case suite_node:	return F("suite_node");
	case living_node:	return F("living_node");
	}
	return (F(""));
}


bool Houz::radioReady()
{
	radio->printDetails();
	return radio_status;
};


bool Houz::radioRead()
{
	//if radio is not enabled, discard anything
	if (!radio_status) { return false; };
	uint8_t _radioNode;
	if (!radio->available(&_radioNode)) { return false; };

	statusLedBlink();

	//get payload
	unsigned long _radioPayLoad;
	while (radio->available()) {
		radio->read(&_radioPayLoad, sizeof(unsigned long));
	}

	//decode payload
	deviceData device = codec->decode(_radioPayLoad, _radioNode);

	//prepare for next packet
	radio->startListening();

	//server must notify host
	if (node_id == server_node) {
		console->print(F("["));
		console->print(action_rfReceived);
		console->print(F("N"));
		console->print(_radioNode, HEX);
		console->print(_radioPayLoad, HEX);
		console->println(F("]"));
		return false;
	}

	//handle pong back command
	if (device.hasData && device.id == node_id && device.cmd == CMD_STATUS) {
		radioSend(CMD_STATUS, device.id, (0xFFFF) & ~device.payload);
		console->println(F("\tpong\tback"));
		return false;
	}
	console->print(F("dev:"));
	console->println(device.id);

	//handle command
	pushData(device);
	return device.hasData;
};


bool Houz::radioSend(deviceData device) {
	return radioSend(device, device.node);
};
bool Houz::radioSend(deviceData device, byte nodeId) {
	return radioSend(device.cmd, device.id, device.payload, nodeId);
};
bool Houz::radioSend(u8 deviceCmd, u8 deviceId, u32 devicePayload) {
	return radioSend(deviceCmd, deviceId, devicePayload, server_node);
};
bool Houz::radioSend(Weather weather){
	if(node_id==suite_node){
		radioSend(CMD_VALUE, suite_enviroment, weather.online);
		if(weather.online){
			radioSend(CMD_VALUE, suite_temp, weather.temp*100);
			radioSend(CMD_VALUE, suite_humidity, weather.hum*100);
			radioSend(CMD_VALUE, suite_pressure, codec->pressureEncode(weather.pressure));
  	  console->print(F("%\tpressure: ")); 
			console->print(codec->pressureEncode(weather.pressure));	
		}
	}
	return true;
};

bool Houz::radioSend(u8 deviceCmd, u8 deviceId, u32 devicePayload, byte nodeId) {
	if (!radio_status) return false;
	if (!server_online) 
		if (!radioSendQueue.isEmpty()) return false;
	if (radioSendQueue.count() > 5) return false;

	//enqueue send
	radioPacket packet;
	packet.message = codec->encode(deviceCmd, deviceId, devicePayload);
	packet.node = nodeId;
	packet.retries = 0;
	radioSendQueue.enqueue(packet);

	if (node_id == server_node) return true;

	console->print(F("\tradioSend\tpush\t"));
	printRadioPacket(packet);
	console->println();
	return true;
};

void Houz::printRadioPacket(radioPacket packet) {
	console->print(F("N"));
	console->print(packet.node, HEX);
	console->print(packet.message, HEX);
};

void Houz::radioWrite() {
	if (!radio_status) { return; };
	if (radio_next_packet > millis()) { return; };

	//peek next packet
	if (radioSendQueue.isEmpty()) { return; };
	radioPacket packet = radioSendQueue.peek();

	//wait when retrying
	if (packet.retries > 0 && packet.nextRetry > millis()) 
		return;

	packet = radioSendQueue.dequeue();

	//open write pipe
	uint64_t writeAddress;
	radio->stopListening();
	switch (node_id) {
	case server_node:
		switch (packet.node) {
		case office_node: writeAddress = rf_office_rx; break;
		case suite_node: writeAddress = rf_suite_rx; break;
		case living_node: writeAddress = rf_living_rx; break;
		}
		radio->openWritingPipe(writeAddress);
	};

	//send
	bool result = 0;
	result = radio->write(&packet.message, sizeof(unsigned long), 0);
	if (!result) {
		packet.retries++;
		packet.nextRetry = millis() + 1000;
		if (packet.retries < 10) {
			radioWriteResult(action_rfSentRetry, packet);
			radioSendQueue.enqueue(packet);
		}
		else {
			radioWriteResult(action_rfSentFail, packet);
			if (node_id != server_node) 
				server_online = false;
		}
	}
	else {
		radioWriteResult(action_rfSentOk, packet);
		if (node_id != server_node && !server_online) 
			server_online = true;
	};

	radio->startListening();
	radio_next_packet = millis() + 200;
}
void Houz::radioWriteResult(byte result, radioPacket packet) {
	
	//server 
	if (node_id == server_node) {
		printToHost(result, packet.node, packet.message);
		return;
	}

	//hosts
	console->print(F("\tradioWrite\t"));
	switch (result)
	{
	case action_rfSentFail: 
		statusLedVoid();
		console->print(F("drop\t"));
		break;
	case action_rfSentRetry: 
		console->print(packet.retries);
		console->print(F(" retry\t"));
		break;
	case action_rfSentOk:
		statusLedBlink();
		console->print(F("ok\t\t"));
		break;
	}
	printRadioPacket(packet);
	console->println();

};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Serial stuff 

bool Houz::serialRead(){
  while (console->available() > 0) {
    int inChar = console->read();
	//console->print((char)inChar);
	if (inChar == '\n' || (char)inChar == '\\') {
		handleCommand(codec->decode(serialBuffer));
	}else{
		serialBuffer += (char)inChar;
	}
  }
  return false;
}

void Houz::handleCommand(deviceData device){
	if (!device.hasData) device.message = serialBuffer;
	serialBuffer = "";

	//ping response
	if (device.id == node_id && device.cmd == CMD_QUERY) {
		console->println(radio_status ? F("[online]") : F("[offline]"));
		return;
	}

	if (node_id == server_node) {
		//server node handling
		if (device.hasData) radioSend(device);
	}
	else {
		//client nodes
		pushData(device);
	}
};
 


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// IO stuff | shifted outputs

void Houz::setIo(u32 io, bool status) {
	//set bit
	if(status)
		bitSet(ioStatus, io);
	else
		bitClear(ioStatus, io);

	//push
	ioRender();
};
void Houz::setIo(word ioRawValue) {
	ioStatus = ioRawValue;
	ioRender();
};

bool Houz::getIo(u32 io) {
	return bitRead(ioStatus, io);
};

void Houz::ioRender() {
	digitalWrite(latchPin, LOW);
	shiftOut(dataPin, clockPin, MSBFIRST, (ioStatus >> 8));
	shiftOut(dataPin, clockPin, MSBFIRST, ioStatus);
	digitalWrite(latchPin, HIGH);

	console->print(F(" | ioRender "));
	console->println(ioStatus, BIN);
}

word Houz::getIoStatus() {
	return ioStatus;
}