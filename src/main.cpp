/*
  Blink
  Turns on an LED on for one second, then off for one second, repeatedly.
  Example uses a static library to show off generate_arduino_library().
 
  This example code is in the public domain.
 */
#if ARDUINO >= 100
    #include "Arduino.h"
#else
    #include "WProgram.h"
#endif


#include "dimmer.h"

/** SSR library **/
void setup() {
  // Start the UART
  Serial.begin(115200) ;
  Serial.setTimeout(-1);

  dimmer_init();
  //dimmer_set(PHASE_LEADING_EDGE, CHANNEL1, 200);
}

uint8_t readFromSerial() {
  while(Serial.available() < 1){
    ;
  }
  return Serial.read();
}

void loop() {
  uint8_t messageStart[2];
  uint8_t fire_type;
  uint8_t channel;
  uint8_t value;

  // Protocol: Start each message with 255 (= 0xFF)
  do {
    messageStart[0] = readFromSerial();
    messageStart[1] = readFromSerial();
  } while (messageStart[0] != 0xF6 && messageStart[1] != 0x6F);

  fire_type = readFromSerial();
  channel = readFromSerial();
  value = readFromSerial();

  dimmer_set(fire_type, channel, value);
}

