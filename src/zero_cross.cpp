#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>

#include "zero_cross.h"

// timer1 register definitions
#define TIMER1_CS_MASK                0x7
#define TIMER1_CS_8_PRES              0x2


static void reset_timer(void);
static void start_timer(uint16_t);
static void reset_zc_interrupt(void);
static void channels_init(void);

static uint16_t timer_offset = TIMER_OFFSET_TICKS;

volatile t_wave wave_type = INITIALIZING;
volatile t_ticks wave_width = 0;
volatile t_ticks half_wave_width = 0;

// TODO: remove this static value
volatile t_ticks frac_width;
volatile bool zc_valid = false;


static cb_zero_cross zc_callback;

static void reset_zc_interrupt(void) {
  DDRD &= ~(1<<PD2);

  // fire int0 interrupt on rising edge at digital pin 2
  EICRA |= ((1<<ISC01) | (1<<ISC00));

  // clear pending interrupts
  EIFR |= (1<<INTF0);

  // enable interrupt
  EIMSK |= (1<<INT0);
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

static void set_trigger(t_ticks tick) {
  OCR1A = tick;
  TIMSK1 |= (1<<OCIE1A);
}

static void set_wave_width(t_ticks ticks) {
  wave_width = ticks;

  // divide by 2
  half_wave_width = ticks >> 1;

  // divided by 512
  frac_width = half_wave_width >> 8;

  OCR2A = frac_width;
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

  // clear all pending interrupts
  EIFR |= (1<<INTF0);
  TIFR1 |= ((1<<OCF1A) | (1<<OCF1B) | (1<<TOV1));
}


/**
 * ZC Optocoupler interrupt
 **/
ISR (INT0_vect) {
  // temporarily store changing values
  t_ticks timer_value = read_timer_value();
  uint8_t input_state = (PIND & (1<<PD2));

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

        set_trigger(half_wave_width);

        zc_valid = true;
      }
      break;
    }
    default: {
      // TODO: update wave_width and half_wave_width at every cycle

      // TODO: calculate additional offset which is lost at timer reset
      set_timer_value(timer_offset);

      set_trigger(half_wave_width);

      zc_valid = true;
    }
  }

  sei();
}

/**
 * Timer1 compare A interrupt - zero cross interrupt
 **/
ISR (TIMER1_COMPA_vect) {
  if (wave_type == POSITIVE) {

    if (zc_valid) {
      // let the timer capture the end of the measured wave
      set_trigger(wave_width);

      // call user code for falling edge
      zc_callback(FALLING_EDGE, 0);

      wave_type = NEGATIVE;
    } else {
      reset();
    }

    // check if a reasonable wave length has been measured
    // otherwise restart with calibration

  } else if (wave_type == NEGATIVE) {
    // let the timer capture the half of the wave
    set_trigger(half_wave_width);

    // call user code for rising edge
    zc_callback(RISING_EDGE, wave_width);

    wave_type = POSITIVE;

    zc_valid = false;
  }
}

ISR (TIMER1_OVF_vect) {
  reset();
}

void zc_start(cb_zero_cross _zc_callback) {
  reset();

  reset_timer();
  reset_zc_interrupt();

  zc_callback = _zc_callback;
}