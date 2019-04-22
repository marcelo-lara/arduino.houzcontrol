/*
Name:		external_sensors.ino
Created:	05-Apr-17 21:42:21
Author:	DarkAngel
*/

#include <Houz.h>

//light sensor
#define lightSensorPin		A0

//bosch bme280 weather module (3.3v)
#include <HouzWeather.h>
#define bme280_SDA			A4
#define bme280_SCL			A5
HouzWeather houzWeather(bme280_SDA, bme280_SCL);

//lights
#define ceilingLightPin		8
#define ceilingLightSwitch	7
#define statusLed			6

//ir control
#include <IRremote.h>
#include <IRremoteInt.h>
//#define irRecvPin  6 //IRM-8601S
//IRrecv irrecv(irRecvPin);
#define irSndPin			3 //IR Led (can't be changed)
IRsend irsend;

//Houz/Radio Setup
#define rfCE				9 //RF pin 3 (CE)
#define rfCS				10//RF pin 4 (CS)
RF24 radio(rfCE, rfCS);
Houz houz(office_node, radio, statusLed, Serial);


///////////////////////////
//functional
bool lightOn = 0;
bool airConditionerOn = 0;
int  airConditionerTemp = 24;

void setup() {
	Serial.begin(115200);
	houz.setup();

	//light sensor setup
	pinMode(lightSensorPin, INPUT);

	//ceiling light
	houz.inSwitchSetup(inSwitch);
}
void loop() {
	if (!houz.hasData()) continue;
	deviceData device = houz.getData();
	
	switch (device.id) {

	case suite_node:
		switch (device.cmd) {
		case CMD_QUERY:
			houz.radioSend(houzWeather.getWeather());
			houz.radioSend(CMD_VALUE, office_light, getMainLight() ? 1 : 0);
			break;

		case CMD_SET:
			switch (device.payload)
			{
			//wall switch
			case swSingleClick:
				houz.pushData(CMD_SET, office_light, 2);
				break;

			case swLongPress:
				houz.pushData(CMD_SET, office_node, scene_Sleep);
				houz.radioSend(CMD_SET, server_node, scene_Sleep);
				break;

			//scene handling
			case scene_Sleep:
				houz.pushData(CMD_SET, office_light, 0);
				break;

			case scene_Goodbye:
				houz.pushData(CMD_SET, office_light, 0);
				//houz.pushData(CMD_SET, office_AC, 0);
				break;
			}
		}
		break;

	//weather
	case external_weather:
		Serial.println(F("ext_weather"));
		houz.radioSend(houzWeather.getWeather());
		break;

	case external_light:
		Serial.println(F("ext_light"));
		houz.radioSend(CMD_VALUE, external_light, lightLevel());
		break;

	//appliances
	case office_light:
		Serial.println(F("light"));
		if (device.cmd == CMD_SET) setLight(device.payload == 1);
		houz.radioSend(CMD_VALUE, office_light, lightOn? 1 : 0);
		break;

	// case office_AC:
	// 	Serial.println(F("ac"));
	// 	if (device.cmd == CMD_SET) {
	// 		airConditionerOn = (device.payload == 1);
	// 		ACsendCommand(airConditionerOn ? acBghPowerOn : acBghPowerOff);
	// 	}
	// 	houz.radioSend(CMD_VALUE, office_AC, airConditionerOn ? 1 : 0);
	// 	break;

	default:
		Serial.print(F("??\t"));
		Serial.println(device.raw);
		break;
	}
};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Sensors
u32 lightLevel() {
	return analogRead(lightSensorPin);
};



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// I/O
bool setLight(bool status) {
	lightOn = status;
	digitalWrite(ceilingLightPin, status ? LOW : HIGH);

	Serial.print(F("light\t"));
	Serial.println((lightOn) ? F("on") : F("off"));
	return true;
};



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// infrared
// void ACsendCommand(unsigned long acCode) {
// 	//store status
// 	switch (acCode)
// 	{
// 	case acBghPowerOn:
// 		airConditionerOn = 1;
// 	case acBghPowerOff:
// 		airConditionerOn = 0;
// 	default:
// 		break;
// 	}

// 	//send command
// 	for (int i = 0; i < 3; i++) {
// 		irsend.sendLG(acCode, 28);
// 		delay(100);
// 	}
// }

// unsigned long ACtempCode(u8 temp) {
// 	switch (temp)
// 	{
// 	case 18: return acBghTemp18;
// 	case 19: return acBghTemp19;
// 	case 20: return acBghTemp20;
// 	case 21: return acBghTemp21;
// 	case 22: return acBghTemp22;
// 	case 23: return acBghTemp23;
// 	case 24: return acBghTemp24;
// 	case 25: return acBghTemp25;
// 	case 26: return acBghTemp26;
// 	default: return acBghTemp24;
// 	}
// }

