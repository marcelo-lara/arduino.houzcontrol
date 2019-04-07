#pragma once
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Devices Config

#include <HouzDevicesModel.h>
#include <HouzDevices.h>
#include <HouzDevicesCodec.h>

#include <HouzIrCodes.h>
#include <QueueArray.h>

#include <RF24.h>

// #include <IRremote.h>
// #include <IRremoteInt.h>

// commands
#define CMD_QUERY			0xA
#define CMD_VALUE			0xB
#define CMD_SET				0xC
#define CMD_EVENT			0xD
#define CMD_STATUS			0xE

// scenes
#define scene_Goodbye	0xCF0
#define scene_Sleep		0xCF1
#define scene_Hello		0xCF2

// sw button
#define swSingleClick	0xA01
#define swDoubleClick	0xA02
#define swLongPress		0xA10
#define swClickLong		0xA11

// Server Contract //////////////////////////////////////////////////////////////////////////////////////////////////
#define action_log				0x00
#define action_rfSentOk			0x01
#define action_rfSentRetry		0x02
#define action_rfSentFail		0x03
#define action_rfReceived		0x04
#define action_irReceived		0x10

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Declarations

struct Node
{
	byte id;
	RF24 &radio;
	Stream &serial;

	//shift out
	u8 dataPin;
	u8 latchPin; 
	u8 clockPin;

	//button
	byte mainSwitch;
	byte statusLed;
};

class Houz{
public:
	Houz(byte NodeId, RF24 &_radio, byte _statusLed, Stream &serial);
	Houz(byte NodeId, RF24 &_radio, byte _statusLed, Stream &serial, u8 _dataPin, u8 _latchPin, u8 _clockPin);
	Houz(byte NodeId, RF24 &_radio, byte _statusLed, u8 _inSwitchPin, Stream &serial, u8 _dataPin, u8 _latchPin, u8 _clockPin);
	Houz(Node _node);
	void setup();

	//commands
	bool hasData();
	deviceData getData();
	void pushData(deviceData device);
	void pushData(u8 deviceCmd, u8 deviceId, u32 devicePayload);

	//TODO: move out
	//radio 
	bool radioReady();
	bool radioSend(deviceData device);
	bool radioSend(deviceData device, byte nodeId);
	bool radioSend(u8 deviceCmd, u8 deviceId, u32 devicePayload);
	bool radioSend(u8 deviceCmd, u8 deviceId, u32 devicePayload, byte nodeId);
	bool radioSend(Weather weather);

	//serial
	bool serialRead();
	deviceData serialData();

	//output
	void setIo(u32 io, bool status);
	void setIo(word ioRawValue);
	word getIoStatus();
	bool getIo(u32 io);

	//helpers
	void statusLedBlink();
	void statusLedVoid();

private:
	HouzDevicesCodec* codec;
	void rfSetup(byte NodeId, RF24 &_radio, byte _statusLed, Stream & serial);
	void ioSetup(u8 dataPin, u8 latchPin, u8 clockPin);
	Anim statusLedAnim;
	void statusLedRender();
	int statusLedLevel;

	//wall button
	byte inSwitch;
	void inSwitchSetup();
	void inSwitchUpdate();
	bool inSw_lastStatus;
	unsigned long inSw_lastMs;
	unsigned long inSw_downMs;
	
	int inSwitchCount;
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
	byte statusLed;
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

