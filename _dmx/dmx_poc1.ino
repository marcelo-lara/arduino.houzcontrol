#include <lib_dmx.h> 
DMX_Master dmx_master(100, 2);


void setup() 
{
  for (int i = 8; i < 14; i++)
  {
	  pinMode(i, INPUT_PULLUP);
  }
  dmx_master.enable();
}

int upval=255;
unsigned long nextPrint;
void loop()
{
  int ch;
  for (int i = 8; i < 14; i++)
  {
    dmx_master.setChannelValue (ch, digitalRead(i)==0?0:upval);      
    ch++;
  }
  delay(1000);
}
