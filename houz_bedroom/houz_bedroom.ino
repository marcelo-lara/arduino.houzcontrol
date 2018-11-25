/*
Reference
- RF nRF24L01+
http://blog.brunodemartino.com.ar/2013/11/arduino-raspberry-pi-talking-wirelessly/
wiring: D7>CS | D8>CE | (sck) D13>5 | (mosi) D11>6 | (miso) D12>7

- IR Receiver
IRM-8601S

*/

#include <EEPROM.h>
#include <QueueArray.h>
#include <HouzInfrared.h>
#include <HouzDevices.h>
#include <IRremote.h>

//hardware setup
#define irRecvPin	6	//IRM-8601S
#define irSndPin	3	//IR Led (can't be changed)
#define inSwitch	A0	//wall switch
#define swLight		5	//wall led indicator
#define swLightLvl	200	//wall led indicator level
#define lightOut	2	//relay


//functional
#define lightOnAddr	0
bool lightOn = 0;
bool airConditionerOn = 0;
int  airConditionerTemp = 24;
bool tvOn = 0;

//TODO: dim wallSwlight
//ir setup
IRrecv irrecv(irRecvPin);
IRsend irsend;

//radio setup
#define rfRecvLed 9 //RF Led
#define rfCE 8      //RF pin 3 (CE)
#define rfCS 7      //RF pin 4 (CS)
RF24 radio(rfCE, rfCS);
HouzDevices houz(bedroom_node, radio, swLight, Serial);

void setup() {
	Serial.begin(115200);
	irrecv.enableIRIn();	//ir setup
	houz.setup();		//houz setup

	//switch setup
	pinMode(inSwitch, INPUT_PULLUP);
	pinMode(lightOut, OUTPUT);
	lightOn = EEPROM.read(lightOnAddr);
	setCeilingLight(lightOn);
}

void loop()
{
	if (houz.hasData()) handleCommand(houz.getData()); 
	infraredRead();  //handle IR
	switchRead();   //lightSwitch touch
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Handle Commands

void handleCommand(deviceData device) {
	switch (device.id) {
	case bedroom_light:
		Serial.println("[bedroom_light]");
		if (device.cmd == CMD_SET) setCeilingLight(device.payload == 1);
		houz.radioSend(CMD_VALUE, bedroom_light, lightOn ? 1 : 0);
		break;

	case bedroom_AC:
		Serial.println("[bedroom_AC]");
		if (device.cmd == CMD_SET) ACsendCommand(device.payload == 1 ? acBghPowerOn : acBghPowerOff);
		houz.radioSend(CMD_VALUE, bedroom_AC, airConditionerOn ? 1 : 0);
		break;

	case bedroom_AC_temp:
		Serial.println("[bedroom_AC_temp]");
		if (device.cmd == CMD_SET) {
			airConditionerTemp = (device.payload);
			ACsendCommand(ACtempCode(airConditionerTemp));
		}
		else
			houz.radioSend(CMD_VALUE, bedroom_AC_temp, airConditionerTemp);
		break;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// IR Remote Control
void infraredRead() {
	decode_results results;
	if (irrecv.decode(&results)) {  // Grab an IR code
		if (results.value != 0xFFFFFFFF) {
			Serial.print("\nIR.receive> ");
			handleIrCode(results.value);
			houz.statusLedBlink();
		}
		irrecv.resume(); // Prepare for the next value
	}
}
void handleIrCode(unsigned long irCode) {

	if (irCode == 0xFFFFFFFF) { return; }
	switch (irCode)	{

	//turn light on
	case irDvrCenter:
		setCeilingLight(!lightOn);
		houz.radioSend(CMD_EVENT, bedroom_ir, bedroom_light);
		break;

	//turn on/off AC
	case irDvrA:
		airConditionerOn = !airConditionerOn;
		ACsendCommand(airConditionerOn ? acBghPowerOn : acBghPowerOff);
		houz.radioSend(CMD_EVENT, bedroom_switch, bedroom_AC);
		break;

		//TODO: AC temp up
		//TODO: AC temp down

	//TV Power
	case tvPower:
		Serial.println("IR snd> tvPower");
		irsend.sendLG(tvPower, 28);
		irrecv.enableIRIn();
		break;

	case 0x6B59: //down
		Serial.println("down");
		break;

	default:
		Serial.print("unknown: 0x");
		houz.radioSend(CMD_EVENT, bedroom_ir, irCode);
		Serial.println(irCode, HEX);
		break;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Wall switch
unsigned long switchReadSt = 0;
void switchRead() {
	if (switchReadSt > millis()) return; //debounce

	// check button
	if (digitalRead(inSwitch) == HIGH) return;
	Serial.println("switch> pressed..");
	setCeilingLight(!lightOn);

	// notify
	houz.radioSend(CMD_EVENT, bedroom_switch, bedroom_light);
	switchReadSt = millis() + 500;
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Ceiling Light
void setCeilingLight(bool status) {
	if (lightOn == status) return;
	lightOn = status;
	digitalWrite(lightOut, lightOn? LOW: HIGH);

	EEPROM.write(lightOnAddr, lightOn);
	houz.radioSend(CMD_VALUE, bedroom_light, status ? 1 : 0);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Air Conditioner

void ACsendCommand(unsigned long acCode) {
	//store status
	if (acCode == acBghPowerOn) { 
		airConditionerOn = 1; 
		houz.radioSend(CMD_VALUE, bedroom_AC, 1);
	}
	if (acCode == acBghPowerOff) { 
		airConditionerOn = 0; 
		houz.radioSend(CMD_VALUE, bedroom_AC, 0);
	}

	//send command
	for (int i = 0; i < 3; i++) {
		irsend.sendLG(acCode, 28);
		delay(50);
	}
	irrecv.enableIRIn();

}
unsigned long ACtempCode(u8 temp) {
	switch (temp)
	{
	case 18: return acBghTemp18;
	case 19: return acBghTemp19;
	case 20: return acBghTemp20;
	case 21: return acBghTemp21;
	case 22: return acBghTemp22;
	case 23: return acBghTemp23;
	case 24: return acBghTemp24;
	case 25: return acBghTemp25;
	case 26: return acBghTemp26;
	default: return acBghTemp24;
	}
}

