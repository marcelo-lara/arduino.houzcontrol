#pragma once

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

	byte *steps;

	unsigned long nextStep;
};

typedef struct Weather{
public:
  bool online;
	float temp;
	float hum;
	float pressure;
	float alt;
};
