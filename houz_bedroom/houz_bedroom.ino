
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
	pinMode(inSwitch, INPUT_PULLUP);
	pinMode(mainLight, OUTPUT);

  //io setup
	houz.setIo(0);

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

 // main light | N2DC250001
 	case suite_light:
		Serial.println("[suite_light] ");
		if (device.cmd == CMD_SET) setMainLight(device.payload);
    houz.radioSend(CMD_EVENT, suite_light, device.payload);
		break;

 // AC control | N2DC230001
 	case suite_AC:
		Serial.println("[suite_AC] ");
		if (device.cmd == CMD_SET) setAC(device.payload, 24);
    houz.radioSend(CMD_EVENT, suite_AC, device.payload);
		break;

 // ceiling fan | 
 	case suite_fan:
		Serial.println("[suite_fan] ");
		if (device.cmd == CMD_SET) setMainLight(device.payload);
    houz.radioSend(CMD_EVENT, suite_fan, device.payload);
		break;

 // weather | N2DA200000
  case suite_enviroment:
    Weather cond = houzWeather.getWeather();
    houz.radioSend(CMD_VALUE, suite_enviroment, cond.online);
    if(cond.online){
      houz.radioSend(CMD_VALUE, suite_temp, cond.temp);
      houz.radioSend(CMD_VALUE, suite_humidity, cond.hum);
      houz.radioSend(CMD_VALUE, suite_pressure, cond.pressure);
    }
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
	case sonyIrDvrSelect:	Serial.println("DvrSelect"); houz.pushData(CMD_SET, suite_light, 2); break;

	//fan control
	case sonyIrDvr1: Serial.println("dvr1"); houz.pushData(CMD_SET, suite_fan, 1); break;
	case sonyIrDvr2: Serial.println("dvr2"); houz.pushData(CMD_SET, suite_fan, 2); break;
	case sonyIrDvr3: Serial.println("dvr3"); houz.pushData(CMD_SET, suite_fan, 3); break;
	case sonyIrDvr4: Serial.println("dvr4"); houz.pushData(CMD_SET, suite_fan, 4); break;
	case sonyIrDvr0: Serial.println("dvr0"); houz.pushData(CMD_SET, suite_fan, 0); break;
  case sonyIrDvrEnter: Serial.println("dvrEnter"); houz.pushData(CMD_GET, suite_enviroment, 0); break;

	//unknown code
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
	houz.pushData(CMD_SET, suite_light, 2);
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
	houz.getIoStatus();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// AC control
void setAC(bool state, int temp){
	Serial.print("AC\t");
	Serial.print(state==1?"on":"off");
	Serial.print("\t");
	Serial.println(temp);
	irsend.sendLG(state?acBghPowerOn:acBghPowerOff, 28);
	//irsend.send(0x1035C9DA,32);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Enviroment sensor | BME280
void getWeather(){
    weather_dump(houzWeather.getWeather());
}

void weather_dump(Weather cond){
  Serial.print(F("bme280: ")); 
  Serial.print(cond.online?"online":"offline"); 
  Serial.print(F("\ttemp: ")); 
  Serial.print(cond.temp);
  Serial.print(F("°C\thum: ")); 
  Serial.print(cond.hum);
  Serial.print(F("%\tpressure: ")); 
  Serial.print(cond.pressure);
  Serial.print(F("hPa\tAltitude[m]: ")); 
  Serial.println(cond.alt);
}