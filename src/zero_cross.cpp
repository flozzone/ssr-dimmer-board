#include <avr/io.h>
#include <avr/interrupt.h>

//TODO: remove this
#include <stdio.h>
#define DEBUG(msg) printf(msg "\n\r")
//#define DEBUG(msg)

#include "zero_cross.h"

// timer1 timings
#define TIMER1_PRESCALER              8
#define TICK_TIME_NS                  ((TIMER1_PRESCALER*1000)/(F_CPU/1000000))
#define NS_TO_TICKS(ns)               (ns / TICK_TIME_NS)
#define US_TO_TICKS(us)               (NS_TO_TICKS(us * 1000))
#define MS_TO_TICKS(ms)               (NS_TO_TICKS(ms * 1000000))

#define TIMER_OFFSET_TICKS            (TIMER_OFFSET_NS / TICK_TIME_NS)

// timer1 register definitions
#define TIMER1_CS_OFFSET              0
#define TIMER1_CS_MASK                0x3
#define TIMER1_CS_8_PRES              0x2


static void reset_timer(void);
static void start_timer(uint16_t);
static void reset_zc_interrupt(void);
static void init_outputs(void);

typedef struct {
  uint8_t zc_action;
  uint16_t event_ticks;
} t_setting;

static uint16_t timer_offset = TIMER_OFFSET_TICKS;

volatile t_setting channel_settings[CHANNEL_COUNT];
volatile uint8_t wave_type = INITIALIZING;
volatile uint16_t wave_width = 0;
volatile uint16_t half_wave_width = 0;
volatile uint16_t frac_width = 0;
volatile uint8_t zc_out = 0;
volatile bool zc_valid = false;

static void reset_zc_interrupt(void) {
  DDRD &= ~(1<<PD2);

  // fire int0 interrupt on rising edge at digital pin 2
  EICRA |= ((1<<ISC01) | (1<<ISC00));

  // clear pending interrupts
  EIFR |= (1<<INTF0);

  // enable interrupt
  EIMSK |= (1<<INT0);
}

static void disable_zc_interrupt(void) {
  EIMSK &= ~(1<<INT0);
}

static void init_outputs() {
  // test output
  PORTB &= ~((1<<PB0) | (1<<PB1));
  DDRB |= ((1<<PB0) | (1<<PB1));
}

static void set_output(uint8_t value) {
  if (value > 0) {
    PORTB |= (1<<PB0);
  } else {
    PORTB &= ~(1<<PB0);
  }
}

static void set_output2(uint8_t value) {
  if (value > 0) {
    PORTB |= (1<<PB1);
  } else {
    PORTB &= ~(1<<PB1);
  }
}

static void reset_timer(void) {

  // disable timer at all
  TCCR1A = 0;
  TCCR1B = 0;
  TCCR1C = 0;

  // clear all pending interrupts
  TIFR1 |= ((1<<OCF1A) | (1<<OCF1B) | (1<<TOV1));

  // reenable timer
  TCCR1A = 0;
  TCCR1B = (TIMER1_CS_MASK & TIMER1_CS_8_PRES);
  TCCR1C = 0;

  OCR1A = 0;
  OCR1B = 0;

  // enable overflow interrupt for fall back
  TIMSK1 |= (1<<TOIE1);
}

static void disable_compare_interrupts(void) {
  TIFR1 |= ((1<<OCF1A) | (1<<OCF1B));
  TIMSK1 &= ~((1<<OCIE1A) || (1<<OCIE1B));
}

static void compare_A(uint16_t tick) {
  OCR1A = tick;
  TIMSK1 |= (1<<OCIE1A);
}

static void compare_B(uint16_t tick) {
  OCR1B = tick;
  TIMSK1 |= (1<<OCIE1B);
}

static void set_wave_width(uint16_t ticks) {
  wave_width = ticks;
  half_wave_width = ticks / 2;

  frac_width = wave_width / 256;
}

static void zero_cross(uint8_t edge_type) {

  if (edge_type == RISING_EDGE) {
    set_output(1);
  } else {
    set_output(0);
  }

  for (int i = 0; i < CHANNEL_COUNT; i++) {
    //TODO: perform all zero cross events
  }
}

static uint16_t read_timer_value(void) {
  uint8_t sreg;
  uint16_t value;

  // save global interrupt flag
  sreg = SREG;

  // disable interrupts
  cli();

  // read timer value
  value = TCNT1;

  // restore global interrupt flag
  SREG = sreg;

  return value;
}

static uint16_t set_timer_value(uint16_t value) {
  uint8_t sreg;

  // save global interrupt flag
  sreg = SREG;

  // disable interrupts
  cli();

  // write timer value
  TCNT1 = value;

  // restore global interrupt flag
  SREG = sreg;
}

static void reset(void) {
  disable_compare_interrupts();

  wave_type = INITIALIZING;
  zc_valid = false;

  init_outputs();

  //TODO: disable all outputs
  set_output(0);
}

/**
 * ZC Optocoupler interrupt
 **/
ISR (INT0_vect) {
  // temporarily store changing values
  uint16_t timer_value = read_timer_value();
  uint8_t input_state = (PIND & (1<<PD2));

  set_output2(1);

  cli();

  switch (wave_type) {
    case INITIALIZING: {
      set_timer_value(0);
      wave_type = CALIBRATING;
      break;
    }
    case CALIBRATING: {
      // AC wave length needs to be greater than 15ms
      if (timer_value < MS_TO_TICKS(15)) {
        reset();
      } else {
        wave_type = POSITIVE;

        set_timer_value(timer_offset);

        set_wave_width(timer_value);

        compare_A(half_wave_width);

        zc_valid = true;
      }
      break;
    }
    default: {
      // TODO: update wave_width and half_wave_width at every cycle

      // TODO: calculate additional offset which is lost at timer reset
      set_timer_value(timer_offset);

      compare_A(half_wave_width);

      zc_valid = true;
    }
  }

  sei();

  set_output2(0);
}

/**
 * Timer1 compare A interrupt - zero cross interrupt
 **/
ISR (TIMER1_COMPA_vect) {
  if (wave_type == POSITIVE) {
    // let the timer capture the end of the measured wave
    compare_A(wave_width);

    // call user code for falling edge
    zero_cross(FALLING_EDGE);

    wave_type = NEGATIVE;

    // check if a reasonable wave length has been measured
    // otherwise restart with calibration
    if (zc_valid == false) {
      reset();
    } else {
      zc_valid = false;
    }

  } else if (wave_type == NEGATIVE) {
    // let the timer capture the half of the wave
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
  uint16_t timer_value = read_timer_value();


  for (int i = 0; i < CHANNEL_COUNT; i++) {
    //TODO: test which value is suited more here (when interrupt gets delayed)
    if ((channel_settings[i].event_ticks > (timer_value - 200)) 
        && (channel_settings[i].event_ticks <= timer_value)) {

    }
  }
}

ISR (TIMER1_OVF_vect) {
  reset();
}

void zc_init() {
  reset();

  reset_timer();
  reset_zc_interrupt();
}

void zc_set_channel(uint8_t chan_number, uint8_t zc_action, uint8_t frac) {

    if (chan_number >= CHANNEL_COUNT)
        return;

    channel_settings[chan_number].zc_action = zc_action;
    channel_settings[chan_number].event_ticks = frac * frac_width;
}