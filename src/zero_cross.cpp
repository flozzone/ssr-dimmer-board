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

#define CHANNEL_COUNT                 5
#define MAX_TICKS                     65535


static void reset_timer(void);
static void start_timer(uint16_t);
static void reset_zc_interrupt(void);
static void init_outputs(void);

static uint16_t timer_offset = TIMER_OFFSET_TICKS;

typedef struct {
  bool enabled;
  volatile uint8_t *port;
  volatile uint8_t *ddr;
  volatile uint8_t pin;
  t_action zc_action;
  t_ticks event_ticks;
} t_channel;

/**
 * output ports:
 *  - digital pin 4: PD4
 *  - digital pin 5: PD5
 *  - digital pin 6: PD6
 *  - digital pin 7: PD7
 *  - digital pin 8: PB0
 */
volatile t_channel channels[CHANNEL_COUNT] = {
        {
                .enabled = false,
                .port = &PORTD,
                .ddr = &DDRD,
                .pin = PD4,
                .zc_action = OFF,
                .event_ticks = 0
        },
        {
                .enabled = false,
                .port = &PORTD,
                .ddr = &DDRD,
                .pin = PD5,
                .zc_action = OFF,
                .event_ticks = 0
        },
        {
                .enabled = false,
                .port = &PORTD,
                .ddr = &DDRD,
                .pin = PD6,
                .zc_action = OFF,
                .event_ticks = 0
        },
        {
                .enabled = false,
                .port = &PORTD,
                .ddr = &DDRD,
                .pin = PD7,
                .zc_action = OFF,
                .event_ticks = 0
        },
        {
                .enabled = false,
                .port = &PORTB,
                .ddr = &DDRB,
                .pin = PB0,
                .zc_action = OFF,
                .event_ticks = 0
        }
};


volatile t_wave wave_type = INITIALIZING;
volatile t_ticks wave_width = 0;
volatile t_ticks half_wave_width = 0;

// TODO: remove this static value
volatile t_ticks frac_width = 75;
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
  for (int i = 0; i < CHANNEL_COUNT; i++) {
    *channels[i].port &= ~(1<<channels[i].pin);
    *channels[i].ddr |= (1<<channels[i].pin);
  }
}

static void set_output(t_channel_nr channel, t_action action) {

  if (action > 0) {
    *channels[channel].port |= (1<<channels[channel].pin);
  } else {
    *channels[channel].port &= ~(1<<channels[channel].pin);
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

static void compare_A(t_ticks tick) {
  OCR1A = tick;
  TIMSK1 |= (1<<OCIE1A);
}

static void compare_B(t_ticks tick) {
  OCR1B = tick;
  TIMSK1 |= (1<<OCIE1B);
}

static void set_wave_width(t_ticks ticks) {
  wave_width = ticks;
  half_wave_width = ticks / 2;

  frac_width = wave_width / 256;
}

static inline uint8_t invert(t_action action) {
  return action == ON ? OFF : ON;
}

static void zero_cross(t_edge edge) {

  t_ticks next_event = MAX_TICKS;

  for (int i = 0; i < CHANNEL_COUNT; i++) {
    t_channel *channel = (t_channel *) &channels[i];
    if (channel->enabled) {
      set_output(i, channel->zc_action);

      // find next tick count
      if (channel->event_ticks < next_event) {
        //printf("a %u:%u\n\r", i, channel->event_ticks);
        next_event = channel->event_ticks;
      }
    }
  }

  if (next_event <= MAX_TICKS) {
    compare_B(next_event);
    //printf("n:%u\n\r", next_event);
    //printf("f:%u\n\r", frac_width);
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

  // disable all output channels
  for (int i = 0; i < CHANNEL_COUNT; i++) {
    set_output(i, OFF);
  }
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
  t_ticks timer_value = read_timer_value();
  t_ticks next_event = MAX_TICKS;

  //if (timer_value >= half_wave_width)
  //  timer_value -= half_wave_width;

  for (int i = 0; i < CHANNEL_COUNT; i++) {
    //TODO: test which value is suited more here (when interrupt gets delayed)
    t_channel *channel = (t_channel *) &channels[i];
    if (channel->enabled) {
      // find current event and set output accordingly
      if ((channel->event_ticks > (timer_value - 10))
         && (channel->event_ticks <= timer_value)) {
        set_output(i, invert(channel->zc_action));
      }

      // find next tick count
      if ((channel->event_ticks > timer_value)
          && (channel->event_ticks < next_event)) {
        next_event = channel->event_ticks;
      }
    }
  }

  if (next_event < MAX_TICKS)
    compare_B(next_event);
}

ISR (TIMER1_OVF_vect) {
  reset();
}

void zc_start() {
  reset();

  reset_timer();
  reset_zc_interrupt();
}

void zc_set(t_channel_nr chan_nr, t_action zc_action, uint8_t percent) {
    if (chan_nr < CHANNEL_COUNT) {
      channels[chan_nr].enabled = true;
      channels[chan_nr].zc_action = zc_action;
      channels[chan_nr].event_ticks = percent * frac_width;
      printf("set nr: %u action: %u percent: %u, frac_width: %u ticks: %u\n\r", chan_nr, percent, frac_width, channels[chan_nr].event_ticks);
    }
}

void zc_disable(t_channel_nr chan_nr) {
  if (chan_nr < CHANNEL_COUNT) {
    channels[chan_nr].enabled = false;
  }
}