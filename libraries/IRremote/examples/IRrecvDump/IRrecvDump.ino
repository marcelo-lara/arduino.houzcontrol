/*
 * IRremote: IRrecvDump - dump details of IR codes with IRrecv
 * An IR detector/demodulator must be connected to the input RECV_PIN.
 * Version 0.1 July, 2009
 * Copyright 2009 Ken Shirriff
 * http://arcfn.com
 * JVC and Panasonic protocol added by Kristian Lauszus (Thanks to zenwheel and other people at the original blog post)
 * LG added by Darryl Smith (based on the JVC protocol)
 */

#include <IRremote.h>

int RECV_PIN = 2;

IRrecv irrecv(RECV_PIN);

decode_results results;

void setup()
{
  Serial.begin(115200);
  irrecv.enableIRIn(); // Start the receiver
}

// Dumps out the decode_results structure.
// Call this after IRrecv::decode()
// void * to work around compiler issue
//void dump(void *v) {
//  decode_results *results = (decode_results *)v
void dump(decode_results *results) {
  int count = results->rawlen;

  if(results->value==0xFFFFFFFF){
    Serial.println("\n.. noise ");
    return;
  }

  Serial.print("\n-- decoded ");
  switch(results->decode_type){
    case NEC:       Serial.println("NEC"); break;
    case SONY:      Serial.println("SONY"); break;
    case RC5:       Serial.println("RC5"); break;
    case RC6:       Serial.println("RC6"); break;
    case DISH:      Serial.println("DISH"); break;
    case SHARP:     Serial.println("SHARP"); break;
    case PANASONIC: Serial.println("PANASONIC"); break;
    case JVC:       Serial.println("JVC"); break;
    case SANYO:     Serial.println("SANYO"); break;
    case MITSUBISHI:Serial.println("MITSUBISHI"); break;
    case SAMSUNG:   Serial.println("SAMSUNG"); break;
    case LG:        Serial.println("LG"); break;
    case UNKNOWN:   Serial.println("UNKNOWN"); break;
  }
  Serial.print(results->value, HEX);
  Serial.print(" (");
  Serial.print(results->bits, DEC);
  Serial.println(" bits)");
  Serial.print("Raw (");
  Serial.print(count, DEC);
  Serial.print("): ");

  for (int i = 0; i < count; i++) {
    if ((i % 2) == 1) {
      Serial.print(results->rawbuf[i]*USECPERTICK, DEC);
    } 
    else {
      Serial.print(-(int)results->rawbuf[i]*USECPERTICK, DEC);
    }
    Serial.print(" ");
  }
  Serial.println("");

  dumpCode(results);
}

void  dumpCode (decode_results *results)
{
  int codeType = results->decode_type;
  int codeLen = results->bits;
  unsigned long codeValue = results->value;
  String data_status;
  data_status += F("{\"T\":\"");
  data_status += codeType;
  data_status += F("\",\"D\":[\"");
  data_status += codeValue;
  data_status += F("\",\"");
  for (int i = 1; i < results->rawlen ; i++)
  {
    data_status += results->rawbuf[i] * USECPERTICK;
    if (i != results->rawlen - 1)data_status += ",";
  }
  data_status += F(",\",\"");
  data_status += results->rawlen;

  data_status += F("\"]}");
  Serial.println("-- raw encoded --");
  Serial.println(data_status);
  Serial.println("-----------------");
}

void loop() {
  if (irrecv.decode(&results)) {
    Serial.println(results.value, HEX);
    dump(&results);
    irrecv.resume(); // Receive the next value
  }
}
