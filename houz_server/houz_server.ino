/*
 Module		houz_server.ino
 Created:	05-Apr-17
 Updated:	23-Jan-18
 Author:	DarkAngel
*/
#include <HouzDevices.h>

// radio setup //////////////////////////////////////////////////////////////
//wiring: D8>CS | D9>CE | (sck)D13>5 | (mosi)D11>6 | (miso)D12>7
#define rfRecvLed 10 //RF online Led
#define rfCE 9   //RF pin 3 (CE)
#define rfCS 8  //RF pin 4 (CS)
RF24 radio(rfCE, rfCS);
HouzDevices houz(server_node, radio, rfRecvLed, Serial);

void setup() {
	Serial.begin(115200);
	Serial.println();
	houz.setup();
}

void loop() {
	houz.hasData();
}
