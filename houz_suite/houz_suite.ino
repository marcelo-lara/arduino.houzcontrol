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

#include <IRremote.h>
#include <HouzWeather.h>
#include <Houz.h>

//serial setup
#define serialTx	0	//fixed
#define serialRx	1	//fixed

//radio setup
#define statusLed	   5	//wall led indicator
#define rfCE         9	//RF pin 3 (CE)
#define rfCS	      10	//RF pin 4 (CS)
RF24 radio(rfCE, rfCS);

//ir setup
#define irRecvPin	   2	//IRM-8601S
#define irSndPin	   3	//IR Led (can't be changed)
IRrecv irrecv(irRecvPin);
IRsend irsend;

//bosch bme280 weather module (3.3v)
#define bme280_SDA	A4
#define bme280_SCL  A5
HouzWeather houzWeather(bme280_SDA, bme280_SCL);

//lighting controller setup
#define inSwitch	  A2 	  //wall switch
#define mainLight	  14 	  //ceiling light relay

#define ioClockPin	 8	  //74HC595[11] SH_CP
#define ioLatchPin	 7	  //74HC595[12] ST_CP
#define ioDataPin	   6	  //74HC595[14] DS

Houz houz(suite_node, radio, statusLed, Serial, ioDataPin, ioLatchPin, ioClockPin);

void setup() {
	Serial.begin(115200);
	houz.setup();

  //main light
	pinMode(mainLight, OUTPUT);
	digitalWrite(mainLight,1); //off on restart

  //io setup
	houz.inSwitchSetup(inSwitch);
	houz.setIo(0);

  //ir setup
	irrecv.enableIRIn();

}

void loop() {
	if (houz.hasData()) {handleCommand(houz.getData());}
	infraredRead();
}

void handleCommand(deviceData device) {
	switch (device.id){

// node status
	case suite_node:
		switch (device.cmd)
		{
			case CMD_QUERY:
				houz.radioSend(houzWeather.getWeather());
				houz.radioSend(CMD_EVENT, suite_light, getMainLight()?1:0);
				houz.radioSend(CMD_EVENT, suite_AC, getAC());
				houz.radioSend(CMD_EVENT, suite_fan, getFanSpeed());
				break;
			case CMD_SET:
				switch (device.payload)
				{
					//wall switch
					case swSingleClick:
						houz.pushData(CMD_SET, suite_light, 2);
						break;
					case swLongPress:
						houz.pushData(CMD_SET, suite_node, scene_Sleep);
						houz.radioSend(CMD_SET, server_node, scene_Sleep);
						break;

					//scene handling
					case scene_Sleep:
						houz.pushData(CMD_SET, suite_light, 0);
						break;
					case scene_Goodbye:
						houz.pushData(CMD_SET, suite_light, 0);
						houz.pushData(CMD_SET, suite_fan, 0);
						houz.pushData(CMD_SET, suite_AC, 0);
						break;
				}		
		}
		break;

 // AC control
 	case suite_AC:
		if (device.cmd == CMD_SET) setAC(device.payload);
    houz.radioSend(CMD_EVENT, suite_AC, getAC());
		break;

 // main light
 	case suite_light:
		if (device.cmd == CMD_SET) setMainLight(device.payload);
    houz.radioSend(CMD_EVENT, suite_light, getMainLight()?1:0);
		break;

 // ceiling fan
 	case suite_fan:
		if (device.cmd == CMD_SET) setFanSpeed(device.payload);
    houz.radioSend(CMD_EVENT, suite_fan, getFanSpeed());
		break;

 // weather
  case suite_enviroment:
		houz.radioSend(houzWeather.getWeather());
    break;

	default:		  
		//Serial.println("wtf? " + device.raw);
		break;
	}	  
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// IR Remote Control
void infraredRead() {
  decode_results  results;
  if (!irrecv.decode(&results)) return;
  if (results.value!=0xFFFFFFFF) handleIrCode(results.value);
  irrecv.resume();
}

void handleIrCode(unsigned long irCode) {
	switch (irCode)	{

	//turn light on
	case sonyIrDvrCenter:	houz.pushData(CMD_SET, suite_light, 2); break;

	//fan control
	case sonyIrDvr1: houz.pushData(CMD_SET, suite_fan, 1); break;
	case sonyIrDvr2: houz.pushData(CMD_SET, suite_fan, 2); break;
	case sonyIrDvr3: houz.pushData(CMD_SET, suite_fan, 3); break;
	case sonyIrDvr4: houz.pushData(CMD_SET, suite_fan, 4); break;
	case sonyIrDvr0: houz.pushData(CMD_SET, suite_fan, 0); break;

	//push enviroment
  case sonyIrDvrUp:houz.pushData(CMD_QUERY, suite_enviroment, 0); break;

	//AC
	case sonyIrDvrA: houz.pushData(CMD_SET, suite_AC, 24); break;
	case sonyIrDvrB: houz.pushData(CMD_SET, suite_AC, 0); break;

	//scenes
	case sonyIrPause: houz.pushData(CMD_SET, suite_node, scene_Sleep);

	//unknown code
  default:
		break;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Lighting control
void setMainLight(int state){ //todo: check this..
  if(state>1) state=!digitalRead(mainLight)==0?1:0;
  digitalWrite(mainLight, state==1?0:1);
}

bool getMainLight(){
	return !digitalRead(mainLight);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Fan control
void setFanSpeed(int fanSpeed){
	switch (fanSpeed)
	{
		case 0: houz.setIo(B0); break;
		case 1: houz.setIo(B1000); break;
		case 2: houz.setIo(B0100); break;
		case 3: houz.setIo(B0010); break;
		case 4: houz.setIo(B0001); break;
		default: break;
	}	
}
int getFanSpeed(){
	switch (houz.getIoStatus())
	{
		case 8: return 1; break;
		case 4: return 2; break;
		case 2: return 3; break;
		case 1: return 4; break;
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// AC control
int acStatus = 0;
void setAC(int state){
	if(state=1) state=24;
	irsend.sendLG(state>0?acBghPowerOn:acBghPowerOff, 28);
	acStatus=state;
}

int getAC(){
	return acStatus;
}
