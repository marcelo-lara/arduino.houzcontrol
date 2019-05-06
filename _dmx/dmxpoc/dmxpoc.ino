#include <lib_dmx.h>  // comment/uncomment #define USE_UARTx in lib_dmx.h as needed

#define    DMX512     (0)    // (250 kbaud - 2 to 512 channels) Standard USITT DMX-512
#define    DMX1024    (1)    // (500 kbaud - 2 to 1024 channels) Completely non standard - TESTED ok
#define    DMX2048    (2)    // (1000 kbaud - 2 to 2048 channels) called by manufacturers DMX1000K, DMX 4x or DMX 1M ???

void setup() 
{
  Serial.begin(115200);
  Serial.println("--start");

  for (int i = 8; i < 14; i++)
  {
	  pinMode(i, INPUT_PULLUP);
  }
  


  ArduinoDmx1.set_control_pin(22);   // Arduino output pin for MAX485 input/output control (connect to MAX485-1 pins 2-3) 
  ArduinoDmx1.set_tx_address(1);    // set rx1 start address
  ArduinoDmx1.set_tx_channels(512); // 2 to 2048!! channels in DMX1000K (512 in standard mode) See lib_dmx.h  *** new *** EXPERIMENTAL
  ArduinoDmx1.init_tx(DMX512);    // starts universe 1 as rx, standard DMX 512 - See lib_dmx.h, now support for DMX faster modes (DMX 1000K)


}//end setup()
int incomingByte = 0;
int upval=255;
unsigned long nextPrint;
void loop()
{
  serialIn();
  int ch;
  for (int i = 8; i < 14; i++)
  {
    ArduinoDmx1.TxBuffer[ch]=digitalRead(i)==0?0:upval;
    ch++;
  }

  if(nextPrint>millis()) return;
  nextPrint=millis()+1000;
  printDmx();

}

String serialBuffer;

void serialIn(){
  while (Serial.available() > 0) {
    int inChar = Serial.read();
		if (inChar == '\n' || (char)inChar == '\\') {
        upval=serialBuffer.toInt();
        serialBuffer="";
		}else{
			serialBuffer += (char)inChar;
			if(serialBuffer.length()>16) serialBuffer="";
		}
  }
}

void parseDmx(String instr){
  Serial.println("parse: ");
  int channel = instr.substring(0,3).toInt();
  int value = instr.substring(3,6).toInt();
  ArduinoDmx1.TxBuffer[channel]=255;
  printDmx();
}

void printDmx(){
  for (int i = 0; i < 16; i++)
  {
    Serial.print("\t");
    Serial.print(ArduinoDmx1.TxBuffer[i]);
  }
  Serial.println("");
}
