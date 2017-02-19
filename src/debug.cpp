//
// Created by floz on 19/02/17.
//

// PB1, PB2

#include "debug.h"
#include <avr/io.h>

bool pA = false;
bool pB = false;

static void set_A(void);
static void set_B(void);

void debug_init(void) {
  set_A();
  set_B();
  DDRB |= ((1<<PB1)|(1<<PB2));
}

static void set_A(void) {
  if (pA) {
    PORTB |= (1<<PB1);
  } else {
    PORTB &= ~(1<<PB1);
  }
}

static void set_B(void) {
  if (pB) {
    PORTB |= (1<<PB2);
  } else {
    PORTB &= ~(1<<PB2);
  }
}

void debug_toggleA(void) {
  pA = !pA;
  set_A();
}

void debug_toggleB(void) {
  pB = !pB;
  set_B();
}