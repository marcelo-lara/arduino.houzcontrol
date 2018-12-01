/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Devices Config

#include <HouzDevicesModel.h>
#include <HouzDevicesNodes.h>
#include <HouzDevicesCodec.h>

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

// Server Contract //////////////////////////////////////////////////////////////////////////////////////////////////
#define action_log				0x00
#define action_rfSentOk			0x01
#define action_rfSentRetry		0x02
#define action_rfSentFail		0x03
#define action_rfReceived		0x04
#define action_irReceived		0x10

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Declarations

class Houz{
public:
	Houz(byte NodeId, RF24 &_radio, byte _rfStatusLed, Stream &serial);
	Houz(byte NodeId, RF24 &_radio, byte _rfStatusLed, Stream &serial, u8 dataPin, u8 latchPin, u8 clockPin);
	void setup();

	//commands
	bool hasData();
	deviceData getData();
	void pushData(deviceData device);

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
	void statusLedBlink();

private:
	HouzDevicesCodec* codec;
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