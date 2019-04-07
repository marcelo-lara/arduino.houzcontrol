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

#include <IRremote.h>
#include <IRremoteInt.h>
#include <HouzIrCodes.h>
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
#define irRecvPin	2	//IRM-8601S
#define irSndPin	3	//IR Led (can't be changed)
IRrecv irrecv(irRecvPin);
IRsend irsend;

//lighting controller setup
#define inSwitch	4 	//wall switch
#define ioClockPin	5	//74HC595 SH_CP: pin 11
#define ioLatchPin	6	//74HC595 ST_CP: pin 12
#define ioDataPin	7	//74HC595 DS: pin 14

//lighting fx
#define fx_dicroOn	0x1
#define fx_dicroOff	0x2


//lighting logic   controller outputs
byte dicroLight;	// ---- ---- XXXX XXXX
byte mainLight;		// ---- --XX ---- ----
byte spotlLight;	// --XX XX-- ---- ----
byte fxLight;		// XX-- ---- ---- ----

//dragon box fx
bool	dungeonMode = false;
bool	dungeonChanged = false;
u32		dungeonPreStatus;

Houz houz(living_node, radio, swLight, inSwitch, Serial, ioDataPin, ioLatchPin, ioClockPin);

void setup() {
	delay(200); //charge wait
	Serial.begin(115200);
	houz.setup();

	//io setup
	pinMode(inSwitch, INPUT_PULLUP);
	houz.setIo(0);

	//ir setup
	irrecv.enableIRIn();	
}

void loop() {
	if (houz.hasData()) 
		handleCommand(houz.getData()); 
	infraredRead();
	animRender();
}

void handleCommand(deviceData device) {
	switch (device.id){

	case living_node		: 
		//TODO: goodbye mode
		//TODO: handle switch 
		
		switch (device.cmd)
		{
			case CMD_SET:
				Serial.print("cmd: 0x");
				Serial.println(device.payload,HEX);

				switch (device.payload)
				{
					case swSingleClick:
						//no lights on > turn on main
						if(dicroLight==0 && mainLight==0){
							mainLightToggle();
							return;
						}

						if(mainLight>0)
							renderLights(living_mainLight,0);
						if(dicroLight>0)
							houz.pushData(CMD_SET, living_fx, fx_dicroOff);	

						return;
						break;

					case swDoubleClick:
						houz.pushData(CMD_SET, living_fx, dicroLight>0?fx_dicroOff:fx_dicroOn);
						return;
						break;

					case swLongPress:
						Serial.println("longpress: set goodbye");
						houz.pushData(CMD_SET, living_node, scene_Goodbye);
						houz.radioSend(CMD_VALUE, living_node, scene_Goodbye);
						break;
				
					case scene_Goodbye:
						if(mainLight>0) renderLights(mainLight, 0);
						if(fxLight>0) renderLights(fxLight, 0);
						if(spotlLight>0) renderLights(spotlLight, 0);
						if(dicroLight==0) {
							
							return;
						}
						houz.pushData(CMD_SET, living_fx, fx_dicroOff);

					default:
						Serial.print("unknown: 0x");
						Serial.println(device.payload, HEX);
						break;
				}
				break;		
			default:
				Serial.println("query status");
				houz.radioSend(CMD_VALUE, living_mainLight, mainLight);
				houz.radioSend(CMD_VALUE, living_dicroLight, dicroLight);
				houz.radioSend(CMD_VALUE, living_fxLight, fxLight);
				houz.radioSend(CMD_VALUE, living_spotLight, spotlLight);
				break;
		}
		break;

	case living_rawRender	: // set outputs to payload
		Serial.println("rawRender");
		if (device.cmd == CMD_SET) houz.setIo(device.payload);
		break;

//////////////////
// APPLIANCES
	case living_AC			:
		Serial.println("ac");
		break;

//////////////////
// LIGHTING
	case living_mainLight: // 2x center light
		Serial.println("[main_light] ");
		if (device.cmd == CMD_SET) {
			if(device.payload==2){
				mainLightToggle();
			}else{
				renderLights(living_mainLight, device.payload);
			}
		};
		houz.radioSend(CMD_VALUE, device.id, mainLight);
		break;

	case living_dicroLight: // dicro array
		Serial.println("[dicroLight] ");
		if (device.cmd == CMD_SET) {
			renderLights(living_dicroLight, device.payload);
			if (dungeonMode) dungeonChanged = true;
		};
		houz.radioSend(CMD_VALUE, device.id, dicroLight);
		break;

	case living_spotLight: // 4x spotlights
		Serial.println("[spotLight] ");
		if (device.cmd == CMD_SET) {
			renderLights(living_spotLight, device.payload);
			if (dungeonMode) dungeonChanged = true;
		};
		houz.radioSend(CMD_VALUE, device.id, spotlLight);
		break;

	case living_fxLight: // 2x lighting fx
		Serial.println("[fxLight] ");
		if (device.cmd == CMD_SET) {
			renderLights(living_fxLight, device.payload);
			if (dungeonMode) dungeonChanged = true;
		}
		houz.radioSend(CMD_VALUE, device.id, fxLight);
		break;


//////////////////
// FX
	case living_fx: // fx controller
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
		Serial.print("unknown: 0x");
		Serial.println(irCode, HEX);
		break;
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Lighting control
void mainLightToggle(){
	renderLights(living_mainLight, (mainLight == B11) ? B00 : B11);
	houz.radioSend(CMD_VALUE, living_mainLight, mainLight);
	if (dungeonMode) dungeonChanged = true;
}


void renderLights(int module, int outSetting) {
	switch (module)	{
	case living_dicroLight: dicroLight = outSetting; break;
	case living_mainLight: mainLight = outSetting; break;
	case living_spotLight: spotlLight = outSetting; break;
	case living_fxLight: fxLight = outSetting; break;
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
	houz.setIo(~out);
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
	0x6fff,	//0: all + fx 1
	0x6cff, //1: main off 
	0x4055, //2: half dicro off
	0x6c3c, //3: point dicro
	0x6c00, //4: dicro off
	0x2000  //5: only spot 4 
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
	case fx_dicroOn: //dicro on
		break;
	case fx_dicroOff: //dicro off
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

	// Serial.print("animRender [");
	// Serial.print(anim.step);
	// Serial.print("/");
	// Serial.print(anim.stepCount);
	// Serial.print("] ");

	//render step
	switch (anim.id)
	{
	case fx_dicroOn: //dicro on
		renderLights(living_dicroLight, dicroOnAnim[anim.step]);
		break;
	case fx_dicroOff: //dicro off
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

	//update current status
	houz.pushData(CMD_QUERY, living_node, 0);

};
