/*
 Name:		living.ino
 Created:	14-Feb-18 20:49:12
 Author:	DarkAngel
 Reference
 - RF nRF24L01+
	wiring: D7>CS | D8>CE | (sck) D13>5 | (mosi) D11>6 | (miso) D12>7

 - IR Receiver
	IRM-8601S

 - IO shift
	74HC595
*/

#include <HouzSonyRemote.h>
#include <HouzIrCodes.h>
#include <IRremote.h>
#include <HouzInfrared.h>
#include <Houz.h>

//serial setup
#define serialTx	1	//fixed
#define serialRx	2	//fixed

//radio setup
#define swLight		10	//wall led indicator
#define rfCE		9	//RF pin 3 (CE)
#define rfCS		8	//RF pin 4 (CS)
RF24 radio(rfCE, rfCS);

//ir setup
#define irRecvPin	4	//IRM-8601S
#define irSndPin	3	//IR Led (can't be changed)
IRrecv irrecv(irRecvPin);
IRsend irsend;

//lighting controller setup
#define inSwitch	4 	//wall switch
#define ioClockPin	5	//74HC595 SH_CP: pin 11
#define ioLatchPin	6	//74HC595 ST_CP: pin 12
#define ioDataPin	7	//74HC595 DS: pin 14

//lighting logic   controller outputs
byte dicroLight;	// ---- ---- XXXX XXXX
byte mainLight;	// ---- --XX ---- ----
byte spotlLight;	// --XX XX-- ---- ----
byte fxLight;	// XX-- ---- ---- ----

//dragon box fx
bool	dungeonMode = false;
bool	dungeonChanged = false;
u32		dungeonPreStatus;


Houz houz(living_node, radio, swLight, Serial, ioDataPin, ioLatchPin, ioClockPin);

void setup() {
	Serial.begin(115200);
	houz.setup();

	//io setup
	pinMode(inSwitch, INPUT_PULLUP);
	houz.setIo(0xFFFF);

	//announce
	//dicroLight	= B11111111;
	//mainLight	= B11;
	//spotlLight	= B1111;
	//fxLight	= B11;
	//renderLights();
	Serial.println("");
}

void loop() {
	if (houz.hasData()) 
		handleCommand(houz.getData()); 
	switchRead();
	animRender();
}

void cmdToStr(deviceData device) {

	String ret = "";
	switch (device.cmd)
	{
	case CMD_SET: ret = ret + ""; break;
	default:
		break;
	}


};

void handleCommand(deviceData device) {
	switch (device.id){

	case living_node		: //ping node
		Serial.println("[living_node] ping");
		houz.radioSend(CMD_VALUE, device.id, !device.payload);
		break;

	case living_switchLed	: // switch led intensity
		Serial.println("[living_switchLed] ");
		if (device.cmd == CMD_SET) houz.setIo(device.payload);
		houz.radioSend(CMD_VALUE, device.id, houz.getIoStatus());
		break;

//////////////////
// APPLIANCES
	case living_AC			:
		Serial.println("[living_AC]");
		break;

	case living_AC_temp		: 
		Serial.println("[living_AC_temp]");
		break;

//////////////////
// LIGHTING
	case living_mainLight: // 2x center light
		Serial.println("[main_light] ");
		if (device.cmd == CMD_SET) {
			renderLights(living_mainLight, device.payload);
			if (dungeonMode) dungeonChanged = true;
		};
		houz.radioSend(CMD_VALUE, device.id, mainLight);
		break;

	case living_dicroLight: // dicro array
		Serial.println("[living_dicroLight] ");
		if (device.cmd == CMD_SET) {
			renderLights(living_dicroLight, device.payload);
			if (dungeonMode) dungeonChanged = true;
		};
		houz.radioSend(CMD_VALUE, device.id, dicroLight);
		break;

	case living_spotLight: // 2x spotlights
		Serial.println("[living_spotLight] ");
		if (device.cmd == CMD_SET) {
			renderLights(living_spotLight, device.payload);
			if (dungeonMode) dungeonChanged = true;
		};
		houz.radioSend(CMD_VALUE, device.id, spotlLight);
		break;

	case living_auxLight: // 2x lighting fx
		Serial.println("[living_auxLight] ");
		if (device.cmd == CMD_SET) {
			renderLights(living_auxLight, device.payload);
			if (dungeonMode) dungeonChanged = true;
		}
		houz.radioSend(CMD_VALUE, device.id, fxLight);
		break;


//////////////////
// FX
	case living_fx: // raw fx controller
		Serial.println("[living_fx] ");
		if (device.cmd == CMD_SET) 
			animSetup(device.payload);
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


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Lighting control
unsigned long switchReadSt = 0;
void switchRead() {
	if (switchReadSt > millis()) return; //debounce

	int buttonState;
	buttonState = digitalRead(inSwitch);
	if (buttonState == HIGH) return;
	
	switchReadSt = millis() + 500;
	Serial.print("[switchRead] inSwitch pressed ");

	//handle status
	renderLights(living_mainLight, (mainLight == B11) ? B00 : B11);

	//notify
	houz.statusLedBlink();
	houz.radioSend(CMD_EVENT, living_switch, 1);
	houz.radioSend(CMD_VALUE, living_mainLight, mainLight);
}

void renderLights(int module, int outSetting) {
	switch (module)	{
	case living_dicroLight: dicroLight = outSetting; break;
	case living_mainLight: mainLight = outSetting; break;
	case living_spotLight: spotlLight = outSetting; break;
	case living_auxLight: fxLight = outSetting; break;
	}

	unsigned long out = 0;
	// out logic    controller outputs
	// dicroLight	---- ---- XXXX XXXX
	// mainLight	---- --XX ---- ----
	// spotlLight	--XX XX-- ---- ----
	// fxLight		XX-- ---- ---- ----
	out = B11 & ~fxLight;
	out = (out << 4) + (B1111 & ~spotlLight);
	out = (out << 2) + (B11 & ~mainLight);
	out = (out << 8) + (B11111111 & ~dicroLight);
	houz.setIo(out);
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Lighting Fx
Anim anim;

int* animSteps;
int animFx = 0;

int dicroOnAnim[] = {
	B00000000, //0
	B00000001, //1
	B00000011, //2
	B00000111, //3
	B00001111, //4
	B00011111, //5
	B00111111, //6
	B01111111, //7
	B11111111  //8
};
int dicroOffAnim[] = {
	B11111111, //0
	B01111111, //1
	B00111111, //2
	B00011111, //3
	B00001111, //4
	B00000111, //5
	B00000011, //6
	B00000001, //7
	B00000000  //8
};

u32 dragonEngage[] = {
	0x0000, //0
	0x8000, //1
	0xC000, //2
	0xC003, //3
	0x4003, //4
	0x3     //5
};


void animSetup(int _anim) {
	Serial.print(":: animSetup ");
	Serial.println(_anim);

	anim.step = 0;
	anim.on = true;
	anim.id = _anim;
	anim.stepInterval = 200;
	anim.stepCount = 8;

	switch (anim.id){
	
	case 0: //clear anim
		anim.on = false;
		break;
	case 0x1: //dicro on
		break;
	case 0x2: //dicro off
		anim.stepInterval = 200;
		break;

	case 0x10: //dungeon engage
		Serial.println("[dungeon mode engage]");
		anim.stepCount = 5;
		anim.stepInterval = 200;
		dungeonMode = true;
		dungeonChanged = false;
		dungeonPreStatus = houz.getIoStatus();
		break;

	case 0x1F: //dragon disengage
		Serial.println("[dungeon mode disengage]");
		anim.on = false;
		if (dungeonChanged) return; 	//leave untouched
		houz.setIo(0xFFFF & ~dungeonPreStatus);	//reset to previous status
		break;

	default:
		Serial.println("<< not handled..");
		anim.on = false;
		break;
	}
};
void animRender() {
	if (!anim.on) return;
	if (millis() < anim.nextStep) return;
	anim.nextStep = millis() + anim.stepInterval;

	Serial.print("animRender [");
	Serial.print(anim.step);
	Serial.print("/");
	Serial.print(anim.stepCount);
	Serial.print("] ");

	//render step
	switch (anim.id)
	{
	case 1: //dicro on
		renderLights(living_dicroLight, dicroOnAnim[anim.step]);
		break;
	case 2: //dicro off
		renderLights(living_dicroLight, dicroOffAnim[anim.step]);
		break;


	//dragon box fx
	case 0x10: //dragon engage
		houz.setIo(0xFFFF & ~dragonEngage[anim.step]);
		break;
	}

	//queue next step
	if (anim.step < anim.stepCount) {
		anim.step++;
		return;
	}

	//anim last step
	anim.on = false;
	anim.step = 0;
	anim.id = 0;



};



void drawBin(u32 inValue) {
	int i;
	for (i = 15; i > -1; i--) {
		Serial.print(bitRead(inValue, i));
	}
	Serial.println("");
};
