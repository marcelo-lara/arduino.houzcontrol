#include <Wire.h>

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
#include <Houz.h>

//serial setup
#define serialTx	0	//fixed
#define serialRx	1	//fixed

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

//bosch bme280 weather module (3.3v)
#define bme280_SDA			A4
#define bme280_SCL			A5
#include <BlueDot_BME280.h>
BlueDot_BME280 bme280 = BlueDot_BME280();
bool	bme280_online = false; 

//lighting controller setup
#define inSwitch	  A2 	//wall switch
#define mainLight	  14 	//ceiling light relay

#define ioClockPin	  8	  //74HC595[11] SH_CP
#define ioLatchPin	  7	  //74HC595[12] ST_CP
#define ioDataPin	  6	  //74HC595[14] DS

Houz houz(suite_node, radio, statusLed, Serial, ioDataPin, ioLatchPin, ioClockPin);

void setup() {
	Serial.begin(115200);
	houz.setup();

  //main light
	pinMode(inSwitch, INPUT_PULLUP);
	pinMode(mainLight, OUTPUT);

  //io setup
	houz.setIo(0);

  //ir setup
	irrecv.enableIRIn();

  // bme280
  	weather_setup();
    weather_dump();

  //
	
}

void loop() {
	if (houz.hasData()) {handleCommand(houz.getData());}
	switchRead();
	infraredRead();
}

void handleCommand(deviceData device) {
	switch (device.id){

 // main light | N2DC250001
 	case suite_light:
		Serial.println("[suite_light] ");
		if (device.cmd == CMD_SET) setMainLight(device.payload);
		break;

 // AC control | N2DC230001
 	case suite_AC:
		Serial.println("[suite_AC] ");
		if (device.cmd == CMD_SET) setAC(device.payload, 24);
		break;

 // ceiling fan | 
 	case suite_fan:
		Serial.println("[suite_fan] ");
		if (device.cmd == CMD_SET) setMainLight(device.payload);
		break;

 // weather
  case suite_enviroment:
    weather_dump();
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
  decode_results  results;
  if (!irrecv.decode(&results)) return;
  if (results.value!=0xFFFFFFFF) handleIrCode(results.value);
  irrecv.resume();
}

void handleIrCode(unsigned long irCode) {
	Serial.print("IR.receive\t");
	switch (irCode)	{

	//turn light on
	case sonyIrDvrSelect:	Serial.println("DvrSelect"); setMainLight(2); break;

	//fan control
	case sonyIrDvr1: Serial.println("dvr1"); setFanSpeed(1); break;
	case sonyIrDvr2: Serial.println("dvr2"); setFanSpeed(2); break;
	case sonyIrDvr3: Serial.println("dvr3"); setFanSpeed(3); break;
	case sonyIrDvr4: Serial.println("dvr4"); setFanSpeed(4); break;
	case sonyIrDvr0: Serial.println("dvr0"); setFanSpeed(0); break;

  case sonyIrDvrEnter: Serial.println("dvrEnter"); weather_dump(); break;

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
	setMainLight(2);
  //houz.pushData(CMD_SET, living_mainLight, 2);
}

void setMainLight(int state){ //todo: check this..
  if(state>1) state=digitalRead(mainLight)==0?0:1;
  digitalWrite(mainLight, !state);
	Serial.print("light\t");
	Serial.println(digitalRead(mainLight));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Fan control
void setFanSpeed(int fanSpeed){
	Serial.print("fan\t");
	Serial.println(fanSpeed);
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
void getFanSpeed(int fanSpeed){
	
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// AC control
void setAC(bool state, int temp){
	Serial.print("AC\t");
	Serial.print(state==1?"on":"off");
	Serial.print("\t");
	Serial.println(temp);
	irsend.sendLG(acBghPowerOff, 28);
	//irsend.send(0x1035C9DA,32);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Enviroment sensor | BME280

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

Weather weather;
bool whather_read(){
  if(!bme280_online) return false;
  weather.temp=bme280.readTempC();
  weather.hum=bme280.readHumidity();
  weather.pressure=bme280.readPressure();
  weather.alt=bme280.readAltitudeMeter();
  return true;
}

void weather_dump(){
  if(!whather_read()) return;
  Serial.print(F("temp: ")); 
  Serial.print(weather.temp);
  Serial.print(F("°C\thum: ")); 
  Serial.print(weather.hum);
  Serial.print(F("%\tpressure: ")); 
  Serial.print(weather.pressure);
  Serial.print(F("hPa\tAltitude[m]: ")); 
  Serial.println(weather.alt);
}