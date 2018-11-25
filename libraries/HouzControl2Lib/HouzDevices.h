/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Devices Config
#include <HouzSonyRemote.h>
#include <HouzIrCodes.h>
#include <QueueArray.h>
#include <RF24.h>

// commands
#define CMD_QUERY			0xA
#define CMD_VALUE			0xB
#define CMD_SET				0xC
#define CMD_EVENT			0xD
#define CMD_STATUS			0xE

// Master
#define server_node			0    //N0DA000000 ping?

// Office
#define office_node			0x1  //N1DC04F0F0
#define office_AC			0x11 //Air Conditioner on/off
#define office_AC_temp		0x12 //Air Conditioner temperature
#define office_light		0x13 //ceiling light | N1DC130001
#define office_switchLed	0x14 //wall switch led
#define office_switch  		0x15 //wall switch
#define office_ir			0x16 //ir 
#define external_light		0x17 //light sensor [0-1024]
#define external_temp		0x1A //temperature [celsius /100] | N1DA1A0000
#define external_humidity	0x1B //humidity [%] | N1DA1B0000
#define external_pressure	0x1C //pressure [hPa /FIX THIS ((x-900) * 100?)] | N1DA1C0000
#define external_weather	0x1F //all devices | N1DA1F0099

// Bedroom
#define bedroom_node		0x2	 //N2DC02F0F0
#define bedroom_AC			0x23 //Air Conditioner on/off
#define bedroom_AC_temp		0x24 //Air Conditioner temperature
#define bedroom_light		0x25 //Ceiling light N2DC250001
#define bedroom_switchLed	0x26 //Wall switch led
#define bedroom_switch		0x27 //Wall switch
#define bedroom_ir			0x28 //ir

// Living
#define living_node			0x3	 //N3DC04F0F0\n
#define living_switch		0x31
#define living_switchLed	0x32 //N3DC320099\n 
#define living_mainLight	0x33 //N3DC330002\n - center
#define living_dicroLight	0x34 //N3DC340000\n - 2x4 dicro array
#define living_spotLight	0x35 //N3DC350000\n - spotlights
#define living_auxLight		0x36 //N3DC360000\n - guidance leds
#define living_fx			0x37 //N3DC370000\n - raw fx controller
#define living_AC			0x38 //N3DC380001\n
#define living_AC_temp		0x39 //N3DC390018\n

// Frontdesk
#define frontdesk_node		0x5

// Server Contract //////////////////////////////////////////////////////////////////////////////////////////////////
#define action_log				0x00

#define action_rfSentOk			0x01
#define action_rfSentRetry		0x02
#define action_rfSentFail		0x03
#define action_rfReceived		0x04

#define action_irReceived		0x10

#define media_serial			0x0
#define media_rf				0x1
#define media_ir				0x2

// device command
struct deviceData {
public:
	u32  raw;
	bool hasData;

	u8	 id;
	u32  payload;

	byte node;
	u8	 cmd;
	u8	 media;

	String message;
};

struct radioPacket {
public:
	u32 message;
	byte node;
	u8 retries;
	unsigned long nextRetry;
};

struct Anim {
public:
	bool on;
	int id;

	byte stepCount;
	byte step;
	int stepInterval;

	unsigned long nextStep;
};


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Declarations

class HouzDevices{
public:
	HouzDevices(byte NodeId, RF24 &_radio, byte _rfStatusLed, Stream &serial);
	HouzDevices(byte NodeId, RF24 &_radio, byte _rfStatusLed, Stream &serial, u8 dataPin, u8 latchPin, u8 clockPin);
	void setup();

	//commands
	bool hasData();
	void pushData(deviceData device);
	deviceData getData();

	deviceData decode(u32 rawData, u32 rfNodeStation);
	unsigned long encode(u8 _cmd, u8 deviceId, u32 devicePayload);

	//radio
	bool radioReady();
	bool radioSend(deviceData device);
	bool radioSend(deviceData device, byte nodeId);
	bool radioSend(u8 deviceCmd, u8 deviceId, u32 devicePayload);
	bool radioSend(u8 deviceCmd, u8 deviceId, u32 devicePayload, byte nodeId);

	//serial
	bool serialRead();
	deviceData serialData();

	//output
	void setIo(u32 io, bool status);
	void setIo(word ioRawValue);
	word getIoStatus();
	bool getIo(u32 io);

	//helpers
	unsigned long StrToHex(char str[]);
	unsigned long StrToHex(String str);
	deviceData decode(String str);
	
	void statusLedBlink();

private:
	void init(byte NodeId, RF24 &_radio, byte _rfStatusLed, Stream & serial);
	Anim statusLedAnim;
	void statusLedRender();

	//commands
	QueueArray<deviceData> commandsQueue;
	void printToHost(byte result, byte node, u32 message);

	//radio
	RF24* radio;
	void radioSetup();
	void setPipes();

	byte node_id;
	String node_name();
	bool radio_status;
	bool server_online;
	byte rfStatusLed;
	unsigned long radio_next_packet;

	bool radioRead();

	void radioWrite();
	void radioWriteResult(byte result, radioPacket packet);

	QueueArray<radioPacket> radioSendQueue;
	void printRadioPacket(radioPacket packet);

	//serial
	Stream* console;
	String serialBuffer;
	void handleCommand(deviceData inCommand);

	//output
	bool ioReady;
	u8 dataPin;
	u8 latchPin;
	u8 clockPin;
	word ioStatus;
	void ioRender();

};