#include "DMXSerial.h"


/*
Simple example of using the DMSSerial library in combination with EE's hardware configuration to provide 4 relay outputs that can be used to show game states.
This is using the Arduino relay board from: https://www.seeedstudio.com/item_detail.html?p_id=2440
 */

#define ch1 13
#define ch2 12
#define ch3 11
#define ch4 10
#define status 9


void setup()
{
    pinMode(ch1, OUTPUT);
    pinMode(ch2, OUTPUT);
    pinMode(ch3, OUTPUT);
    pinMode(ch4, OUTPUT);
    pinMode(status, OUTPUT);
    DMXSerial.init(DMXReceiver, 2);

}

void loop()
{
    if (DMXSerial.noDataSince() > 1000L)
    {
        //No data for 1 second, disable our relays and flash our LED as a warning.
        if ((millis() % 1000L) > 500) digitalWrite(status, LOW);
        //blackout
        analogWrite(ch1, 0);
        analogWrite(ch2, 0);
        analogWrite(ch3, 0);
        analogWrite(ch4, 0);
    }else{

        //Write our first channel to the on board LED for debugging.
        analogWrite(status, DMXSerial.read(5)); //status?

        analogWrite(ch1, DMXSerial.read(1));
        analogWrite(ch2, DMXSerial.read(2));
        analogWrite(ch3, DMXSerial.read(3));
        analogWrite(ch4, DMXSerial.read(4));
    }
}