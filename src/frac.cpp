//
// Created by floz on 18/02/17.
//

#include "frac.h"

#include <avr/io.h>
#include <avr/interrupt.h>

#define TIMER2_CS_MASK                0x7
#define TIMER2_CS_8_PRES              0x2

static volatile uint8_t frac_counter;
cb_frac_tick cb_frac;


// counter value register TCNT2
// output compare register OCR2A, OCR2B
// interrupt mask register TIMSK2
void frac_init(cb_frac_tick cb) {

  cb_frac = cb;

  // CTC mode (clear timer on compare match)
  TCCR2A = (1<<WGM21);
  TCCR2B = (TIMER2_CS_MASK & TIMER2_CS_8_PRES);
}

void frac_reset(void) {
  TCCR2A = 0;
  TCCR2B = 0;
  TIMSK2 = 0;
}

void frac_restart(uint8_t width) {
  // disable interrupt
  TIMSK2 &= ~(1<<OCIE2A);

  OCR2A = width;

  frac_counter = FRAC_COUNTER_MIN;

  // set timer value
  TCNT2 = 0;

  // enable interrupt on compare match channel A
  TIMSK2 |= (1<<OCIE2A);
}


/**
 * Fraction timer.
 *
 * This timer overflows 256 times for the length of a half wave.
 */
ISR (TIMER2_COMPA_vect) {
  if (frac_counter < FRAC_COUNTER_MAX) {
    frac_counter++;
    cb_frac(frac_counter);
  }
}