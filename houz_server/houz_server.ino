/*
 Module		houz_server.ino
 Created:	05-Apr-17
 Updated:	23-Jan-18
 Author:	DarkAngel
*/
#include <Houz.h>

// radio setup //////////////////////////////////////////////////////////////
//wiring: D8>CS | D9>CE | (sck)D13>5 | (mosi)D11>6 | (miso)D12>7
#define statusLed 10 //status led
#define rfCE 9   //RF pin 3 (CE)
#define rfCSN 8  //RF pin 4 (CSN)
RF24 radio(rfCE, rfCSN);
Houz houz(server_node, radio, statusLed, Serial);

int nodes[] = {suite_node, office_node, living_node};
#define nodeCount 3

void setup() {
	Serial.begin(115200);
	Serial.println();
	houz.setup();
}

void loop() {
	if(!houz.hasData()) return;
	deviceData dev = houz.getData();
	switch (dev.payload)
	{
	
	// notify scene
	case scene_Hello:	
	case scene_Sleep:
	case scene_Goodbye:
		setScene(dev.node, dev.payload);
		break;

	//deliver packet to rf
	default:
		if(dev.node==server_node) return;

		//deliver packet to rf
		houz.radioSend(dev);
		break;
	}
}

void setScene(int originNode, unsigned long scene){
	for (int i = 0; i < nodeCount; i++) {
		if(nodes[i]==originNode) continue;
		Serial.print("target\t");
		Serial.println(nodes[i],HEX);
		houz.radioSend(CMD_SET, nodes[i], scene, nodes[i]);
	}
}
