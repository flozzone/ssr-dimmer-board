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


//#include "blink_lib.h"

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

#include <inttypes.h>
#include <avr/io.h>

#ifndef F_CPU
# error "F_CPU required to calculate timings."
#endif

/* TUNABLES */

// starts the timer at the given value when a rising edge zero cross
// is detected. This is used to compensate the slow zero cross circuit.
#define TIMER_OFFSET_NS               650000


/* CONSTANTS */

// timer1 timings
#define TIMER1_PRESCALER              8
#define TICK_TIME_NS                  ((TIMER1_PRESCALER*1000)/(F_CPU/1000000))

#define TIMER_OFFSET_TICKS            (TIMER_OFFSET_NS / TICK_TIME_NS)

// timer1 register definitions
#define TIMER1_CS_OFFSET              0
#define TIMER1_CS_MASK                0x3
#define TIMER1_CS_8_PRES              0x2


void reset_timer(void);
void start_timer(uint16_t);
void init_zc_interrupt(void);
void init_outputs(void);


enum t_wave_type_ {
  INITIALIZING,
  POSITIVE,
  NEGATIVE
} t_wave_type;

enum t_edge_type_ {
  FALLING_EDGE,
  RISING_EDGE
} t_edge_type;

volatile byte wave_type = INITIALIZING;
volatile uint16_t wave_width = 0;
volatile uint16_t half_wave_width = 0;
static uint16_t timer_offset = TIMER_OFFSET_TICKS;
volatile byte zc_out = 0;


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



  printf("TIMER_OFFSET_TICKS: %u\n\r", TIMER_OFFSET_TICKS);
  init_zc_interrupt();
  init_outputs();
  reset_timer();
}

void init_zc_interrupt(void) {
  //DDRD &= ~(1<<PD2);
}

void init_outputs() {
  // test output
  PORTB &= ~(1<<PB0);
  DDRB |= (1<<PB0);
}

void set_output(byte value) {
  if (value > 0) {
    PORTB |= (1<<PB0);
  } else {
    PORTB &= ~(1<<PB0);
  }
}

void reset_timer(void) {
  TCCR1A = 0;
  TCCR1B = 0;
  TCCR1C = 0;

  OCR1A = 0;
  OCR1B = 0;
}

void start_timer(uint16_t offset) {
  TCCR1B = 0;

  TCNT1 = offset;

  TCCR1A = 0;
  TCCR1B = (TIMER1_CS_MASK & TIMER1_CS_8_PRES);
  TCCR1C = 0;
}

bool is_timer_running(void) {
  return TCCR1B != 0;
}

void compare_A(uint16_t tick) {
  OCR1A = tick;
  TIMSK1 |= (1<<OCIE1A);
}

void set_wave_width(uint16_t ticks) {
  wave_width = ticks;
  half_wave_width = ticks / 2;
}

void zero_cross(byte edge_type) {

  if (edge_type = RISING_EDGE) {
    set_output(1);
  } else {
    set_output(0);
  }
}

/**
 * ZC Optocoupler interrupt
 **/
ISR (PCINT2_vect) {
  // temporarily store changing values
  uint16_t timer_value = TCNT1;
  byte input_state = (PIND & (1<<PD2));

  // detect current wave type
  if (input_state > 0) {

    // synchronize timer1 with the AC frequency 
    if (wave_type == INITIALIZING) {

      // start by measuring one cycle
      if (!is_timer_running()) {
        start_timer(0);

      } else {
        wave_type = POSITIVE;
        start_timer(timer_offset);

        set_wave_width(timer_value);

        compare_A(half_wave_width);
      }
    } else {
      // TODO: update wave_width and half_wave_width at every cycle

      // TODO: calculate additional offset which is lost at timer reset
      start_timer(timer_offset);

      compare_A(half_wave_width);
    }
  }
}

/**
 * Timer1 compare A interrupt - zero cross interrupt
 **/
ISR (TIMER1_COMPA_vect) {
  if (wave_type == POSITIVE) {
    // let the timer run to the full measured cycle time
    compare_A(wave_width);

    // call user code for falling edge
    zero_cross(FALLING_EDGE);

    wave_type = NEGATIVE;

  } else {
    // 
    compare_A(half_wave_width);

    // call user code for rising edge
    zero_cross(RISING_EDGE);

    wave_type = POSITIVE;
  }
}

/**
 * Timer1 compare B interrupt
 **/
ISR (TIMER1_COMPB_vect) {
}



void loop() {
    //blink(100); // Blink for a second
    //digitalWrite(ledPin, HIGH);
}
