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


#include "zero_cross.h"

byte ledPin = 13;
byte zcPin =  2;
byte zcOutPin = 8;

void pciSetup(byte pin)
{
  *digitalPinToPCMSK(pin) |= bit (digitalPinToPCMSKbit(pin));  // enable pin
  PCIFR  |= bit (digitalPinToPCICRbit(pin)); // clear any outstanding interrupt
  PCICR  |= bit (digitalPinToPCICRbit(pin)); // enable interrupt for the group
}


#include <stdarg.h>
void _printf(char *fmt, ... ){
  char buf[128]; // resulting string limited to 128 chars
  va_list args;
  va_start (args, fmt );
  vsnprintf(buf, 128, fmt, args);
  va_end (args);
  Serial.print(buf);
}


// we need fundamental FILE definitions and printf declarations
#include <stdio.h>

// create a FILE structure to reference our UART output function
static FILE uartout = {0} ;

// create a output function
// This works because Serial.write, although of
// type virtual, already exists.
static int uart_putchar (char c, FILE *stream)
{
  Serial.write(c) ;
  return 0 ;
}

/** SSR library **/



void setup() {                

 // Start the UART
 Serial.begin(115200) ;

 // fill in the UART file descriptor with pointer to writer.
  fdev_setup_stream (&uartout, uart_putchar, NULL,
  _FDEV_SETUP_WRITE);

  // The uart is the standard output device STDOUT.
  stdout = &uartout ;

  printf("setup\n\r");
    //blink_setup(); // Setup for blinking
  pinMode(ledPin, OUTPUT);
  pinMode(zcPin, INPUT_PULLUP);
  //pinMode(zcOutPin, OUTPUT);
  pciSetup(zcPin);

  zc_init();
}




void loop() {
    //blink(100); // Blink for a second
    //digitalWrite(ledPin, HIGH);
}
