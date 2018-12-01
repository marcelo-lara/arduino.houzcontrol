#ifndef HouzDevicesModel_h
#define HouzDevicesModel_h

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

#endif