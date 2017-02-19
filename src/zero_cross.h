#ifndef __ZERO_CROSS_H__
#define __ZERO_CROSS_H__

#include <inttypes.h>

#ifndef F_CPU
# error "F_CPU required to calculate timings."
#endif

// timer1 timings
#define TIMER1_PRESCALER              8

#define TICK_TIME_NS                  ((TIMER1_PRESCALER*1000L)/(F_CPU/1000000L))
#define NS_TO_TICKS(ns)               (ns / TICK_TIME_NS)
#define US_TO_TICKS(us)               (NS_TO_TICKS(us * 1000L))
#define MS_TO_TICKS(ms)               (NS_TO_TICKS(ms * 1000000L))

// starts the timer at the given value when a rising edge zero cross
// is detected. This is used to compensate the slow zero cross circuit.
#define TIMER_OFFSET_TICKS            US_TO_TICKS(850L)

enum e_wave {
  INITIALIZING,
  CALIBRATING,
  POSITIVE,
  NEGATIVE
};

enum e_edge {
  FALLING_EDGE,
  RISING_EDGE
};


typedef uint8_t t_wave;
typedef uint8_t t_edge;
typedef uint16_t t_ticks;

/**
 * wave_width > 0 when edge is RISING_EDGE
 **/
typedef void (* cb_zero_cross) (t_edge edge, t_ticks wave_width);

//TODO: phasenanschnitt | ZC(off) -- (on)  -- | ZC(off) ...
//TODO: phasenabschnitt | ZC(on)  -- (off) -- | ZC(on)  ...


void zc_start(cb_zero_cross zc_callback);


#endif /* __ZERO_CROSS_H__ */
