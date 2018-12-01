/*
Name:		external_sensors.ino
Created:	05-Apr-17 21:42:21
Author:	DarkAngel
*/

#include <HouzDevices.h>

//light sensor
#define lightSensorPin		A0

//bosch bme280 weather module (3.3v)
#define bme280_SDA			A4
#define bme280_SCL			A5
#include <BlueDot_BME280.h>
BlueDot_BME280 bme280 = BlueDot_BME280();
bool	bme280_online = false;

//lights
#define ceilingLightPin		8
#define ceilingLightSwitch	7
#define ceilingLightLed		6

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
Houz houz(office_node, radio, ceilingLightLed, Serial);


///////////////////////////
//functional
bool lightOn = 0;
bool airConditionerOn = 0;
int  airConditionerTemp = 24;
//////////////////

void weather_setup() {
	bme280.parameter.communication =		0;		//Choose communication protocol
	bme280.parameter.I2CAddress =			0x76;	//Choose I2C Address
	bme280.parameter.sensorMode =			0b11;	//Choose sensor mode
	bme280.parameter.IIRfilter =			0b100;	//Setup for IIR Filter
	bme280.parameter.humidOversampling =	0b101;	//Setup Humidity Oversampling
	bme280.parameter.tempOversampling =		0b101;	//Setup Temperature Ovesampling
	bme280.parameter.pressOversampling =	0b101;	//Setup Pressure Oversampling 
	bme280.parameter.pressureSeaLevel =		1013.25;//default value of 1013.25 hPa
	bme280.parameter.tempOutsideCelsius =	15;		//default value of 15�C
	bme280_online = (bme280.init() ==		0x60);

	Serial.print("bme280: ");
	Serial.println(bme280_online ? "online" : "offline");
};

void setup() {
	Serial.begin(115200);
	weather_setup();	//weather device
	houz.setup();		//communications

	//light sensor setup
	pinMode(lightSensorPin, INPUT);

	//ceiling light
	pinMode(ceilingLightPin, OUTPUT);
	pinMode(ceilingLightSwitch, INPUT_PULLUP);
}
void loop() {
	if (houz.hasData())	handleCommand(houz.getData());
	switchRead(); //lightSwitch touch
}


void handleCommand(deviceData device) {
	switch (device.id) {

	//weather
		
	case external_weather:
		Serial.println(F("[external_weather]"));
		houz.radioSend(CMD_VALUE, external_temp, bme280.readTempC() * 100);
		houz.radioSend(CMD_VALUE, external_humidity, bme280.readHumidity() * 100);
		houz.radioSend(CMD_VALUE, external_pressure, bme280.readPressure() * 100);
		//houz.radioSend(CMD_VALUE, external_light, lightLevel());
		break;

	case external_light:
		Serial.println(F("[external_lightSensor]"));
		houz.radioSend(CMD_VALUE, external_light, lightLevel());
		break;

	case external_temp:
		Serial.println(F("[external_tempSensor]"));
		houz.radioSend(CMD_VALUE, external_temp, bme280.readTempC() * 100);
		break;

	case external_humidity:
		Serial.println(F("[external_humidity]"));
		houz.radioSend(CMD_VALUE, external_humidity, bme280.readHumidity() * 100);
		break;

	case external_pressure:
		Serial.println(F("[external_pressure]"));
		houz.radioSend(CMD_VALUE, external_pressure, bme280.readPressure() * 10);
		break;


	//appliances
	case office_switchLed:
		Serial.println(F("[office_switchLed]"));
		if (device.cmd == CMD_SET) analogWrite(ceilingLightLed, (int)device.payload);
		houz.radioSend(CMD_VALUE, office_switchLed, analogRead(ceilingLightLed));
		break;

	case office_light:
		Serial.println(F("[office_light]"));
		if (device.cmd == CMD_SET) setLight(device.payload == 1);
		houz.radioSend(CMD_VALUE, office_light, lightOn? 1 : 0);
		break;

	case office_AC:
		Serial.println(F("[office_AC]"));
		if (device.cmd == CMD_SET) {
			airConditionerOn = (device.payload == 1);
			ACsendCommand(airConditionerOn ? acBghPowerOn : acBghPowerOff);
		}
		houz.radioSend(CMD_VALUE, office_AC, airConditionerOn ? 1 : 0);
		break;

	case office_AC_temp:
		Serial.println(F("[office_AC_temp]"));
		if (device.cmd == CMD_SET) {
			airConditionerTemp = (device.payload);
			ACsendCommand(ACtempCode(airConditionerTemp));
		}
		houz.radioSend(CMD_VALUE, office_AC_temp, airConditionerTemp);
		break;

	default:
		Serial.print(F("[unhandled] "));
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
void switchRead() {
	if (digitalRead(ceilingLightSwitch) == HIGH) return;
	Serial.println(F("[switch]\tpressed"));

	setLight(!lightOn);

	houz.statusLedBlink();
	houz.radioSend(CMD_EVENT, office_switch, 1);
	delay(500); //debounce
}

bool setLight(bool status) {
	lightOn = status;
	digitalWrite(ceilingLightPin, status ? LOW : HIGH);

	Serial.print(F("[ceilingLight]\t"));
	Serial.println((lightOn) ? F("on") : F("off"));
	houz.radioSend(CMD_VALUE, office_light, lightOn ? 1 : 0);
	return true;
};



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// infrared
void ACsendCommand(unsigned long acCode) {
	//store status
	switch (acCode)
	{
	case acBghPowerOn:
		airConditionerOn = 1;
	case acBghPowerOff:
		airConditionerOn = 0;
	default:
		break;
	}

	//send command
	for (int i = 0; i < 3; i++) {
		irsend.sendLG(acCode, 28);
		delay(100);
	}
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

