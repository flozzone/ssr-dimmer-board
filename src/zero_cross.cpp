#include <avr/io.h>
#include <avr/interrupt.h>

//TODO: remove this
#include <stdio.h>

#include "zero_cross.h"

// timer1 timings
#define TIMER1_PRESCALER              8
#define TICK_TIME_NS                  ((TIMER1_PRESCALER*1000)/(F_CPU/1000000))

#define TIMER_OFFSET_TICKS            (TIMER_OFFSET_NS / TICK_TIME_NS)

// timer1 register definitions
#define TIMER1_CS_OFFSET              0
#define TIMER1_CS_MASK                0x3
#define TIMER1_CS_8_PRES              0x2


static void reset_timer(void);
static void start_timer(uint16_t);
static void init_zc_interrupt(void);
static void init_outputs(void);

volatile uint8_t wave_type = INITIALIZING;
volatile uint16_t wave_width = 0;
volatile uint16_t half_wave_width = 0;
static uint16_t timer_offset = TIMER_OFFSET_TICKS;
volatile uint8_t zc_out = 0;

void zc_init() {
  //TODO: initialize optocoupler interrupt

  printf("TIMER_OFFSET_TICKS: %u\n\r", TIMER_OFFSET_TICKS);
  init_zc_interrupt();
  init_outputs();
  reset_timer();
}

static void init_zc_interrupt(void) {
  //DDRD &= ~(1<<PD2);
}

static void init_outputs() {
  // test output
  PORTB &= ~(1<<PB0);
  DDRB |= (1<<PB0);
}

static void set_output(uint8_t value) {
  if (value > 0) {
    PORTB |= (1<<PB0);
  } else {
    PORTB &= ~(1<<PB0);
  }
}

static void reset_timer(void) {
  TCCR1A = 0;
  TCCR1B = 0;
  TCCR1C = 0;

  OCR1A = 0;
  OCR1B = 0;
}

static void start_timer(uint16_t offset) {
  TCCR1B = 0;

  TCNT1 = offset;

  TCCR1A = 0;
  TCCR1B = (TIMER1_CS_MASK & TIMER1_CS_8_PRES);
  TCCR1C = 0;
}

static bool is_timer_running(void) {
  return TCCR1B != 0;
}

static void compare_A(uint16_t tick) {
  OCR1A = tick;
  TIMSK1 |= (1<<OCIE1A);
}

static void set_wave_width(uint16_t ticks) {
  wave_width = ticks;
  half_wave_width = ticks / 2;
}

static void zero_cross(uint8_t edge_type) {

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
  uint8_t input_state = (PIND & (1<<PD2));

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
