/*
 Name:		houz_bedroom.ino
 Created:	29-JAN-18
 Author:	DarkAngel
 Reference
 - RF nRF24L01+
 - IR Receiver IRM-8601S
 - IO shift	74HC595
 - Enviroment BME280
*/

#include <HouzSonyRemote.h>
#include <HouzIrCodes.h>
#include <IRremote.h>
#include <IRremoteInt.h>
#include <HouzInfrared.h>
#include <Houz.h>

//serial setup
#define serialTx	1	//fixed
#define serialRx	2	//fixed

//radio setup
#define statusLed	5	//wall led indicator
#define rfCE        9	//RF pin 3 (CE)
#define rfCS	   10	//RF pin 4 (CS)
RF24 radio(rfCE, rfCS);

//ir setup
#define irRecvPin	2	//IRM-8601S
#define irSndPin	3	//IR Led (can't be changed)
IRrecv irrecv(irRecvPin);
IRsend irsend;

//bme280 setup

//lighting controller setup
#define inSwitch	  A2 	//wall switch
#define mainLight	  A0 	//ceiling light relay

#define ioClockPin	8	  //74HC595[11] SH_CP
#define ioLatchPin	7	  //74HC595[12] ST_CP
#define ioDataPin	  6	  //74HC595[14] DS

Houz houz(bedroom_node, radio, statusLed, Serial, ioDataPin, ioLatchPin, ioClockPin);

void setup() {
	Serial.begin(115200);
	houz.setup();

	//io setup
	pinMode(inSwitch, INPUT_PULLUP);
	houz.setIo(0xFFFF);

	//ir setup
	irrecv.enableIRIn();	
}

void loop() {
	if (houz.hasData()) {handleCommand(houz.getData());}
	switchRead();
	infraredRead();
}

void handleCommand(deviceData device) {
	switch (device.id){

	case bedroom_light	: // main light
		Serial.println("[bedroom_light] ");
		if (device.cmd == CMD_SET) setMainLight(device.payload);
		break;

	case 0:
		Serial.print("msg: ");
		Serial.println(device.message);
		break;

	default:		  
		Serial.println("[handleCommand] unknown " + device.raw);
		break;
	}				  
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// IR Remote Control
void infraredRead() {
	decode_results results;
	if (irrecv.decode(&results)) {  // Grab an IR code
		if (results.value != 0xFFFFFFFF) handleIrCode(results.value);
		irrecv.resume(); // Prepare for the next value
	}
}

void handleIrCode(unsigned long irCode) {
	Serial.print("\nIR.receive\t");
	switch (irCode)	{

	//turn light on
	case irDvrCenter:
		Serial.println("irDvrCenter");
		houz.pushData(CMD_SET, living_mainLight, 2);
		break;

	default:
		Serial.print("irUnknown: 0x");
		Serial.println(irCode, HEX);
		break;
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Lighting control
unsigned long switchReadSt = 0;
void switchRead() {
	if (switchReadSt > millis()) return; //debounce

	int buttonState;
	buttonState = digitalRead(inSwitch);
	if (buttonState == HIGH) return;
	
	switchReadSt = millis() + 500;
	Serial.println("switchRead\thit");

	//handle status
	houz.pushData(CMD_SET, living_mainLight, 2);
}

void setMainLight(int state){ //todo: check this..
  Serial.print("mainLight\t");
  Serial.println(state);
	int actState = digitalRead(mainLight);

  digitalWrite(mainLight, (state==1)?1:0);
  Serial.print("mainLight\t");
	Serial.println(actState);
}

