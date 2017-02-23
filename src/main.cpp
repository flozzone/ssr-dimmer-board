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

byte ledPin = 13;
byte zcPin =  2;
byte zcOutPin = 8;

void pciSetup(byte pin)
{
  //*digitalPinToPCMSK(pin) |= bit (digitalPinToPCMSKbit(pin));  // enable pin
  //PCIFR  |= bit (digitalPinToPCICRbit(pin)); // clear any outstanding interrupt
  //PCICR  |= bit (digitalPinToPCICRbit(pin)); // enable interrupt for the group
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
  Serial.setTimeout(-1);

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
  //pciSetup(zcPin);

  // thats it!
  // FROM HERE
  dimmer_init();
  dimmer_set(PHASE_TRAILING_EDGE, CHANNEL1, 1);
  // TO HERE


  dimmer_set(PHASE_LEADING_EDGE, CHANNEL2, 200);
  dimmer_set(FULL_WAVE_BURST, CHANNEL3, 128);
  dimmer_set(HALF_WAVE_BURST, CHANNEL4, 64);
}

char uart_getchar(void) {
  loop_until_bit_is_set(UCSR0A, RXC0); /* Wait until data exists. */
  return UDR0;
}

void _read_line(char *buffer, uint8_t len) {
  do {
    *buffer = uart_getchar();
    if (*buffer == 13) {
      printf("this is it\n\r");
      *buffer = '\0';
      return;
    }
    buffer++;
  } while (--len > 0);
  return;
}

// LINE: [CHANNEL_NR] [ON/OFF] [VALUE]
//             1         1        3

#define LINE_LENGTH 8

void loop() {
  char buffer[8];
  printf("start\n\r");

  Serial.readBytes(buffer, 7);
  buffer[7] = '\0';

  //_read_line(buffer, LINE_LENGTH);

  printf("read: %s\n\r", buffer);

  t_channel_nr channel_nr = buffer[0] - '0';
  t_action action = buffer[2] - '0';
  uint8_t value = atoi(&buffer[4]);

  printf("channel: %u action: %u value: %u\n\r", channel_nr, action, value);
}
