#ifndef __ZERO_CROSS_H__
#define __ZERO_CROSS_H__

#include <inttypes.h>

#ifndef F_CPU
# error "F_CPU required to calculate timings."
#endif

// starts the timer at the given value when a rising edge zero cross
// is detected. This is used to compensate the slow zero cross circuit.
#define TIMER_OFFSET_NS               850000

enum e_action {
  OFF,
  ON
};


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

enum e_channel {
    CHANNEL1 = 0,
    CHANNEL2,
    CHANNEL3,
    CHANNEL4,
    CHANNEL5,
};

typedef uint8_t t_action;
typedef uint8_t t_wave;
typedef uint8_t t_edge;
typedef uint8_t t_channel_nr;
typedef uint16_t t_ticks;

//TODO: phasenanschnitt | ZC(off) -- (on)  -- | ZC(off) ...
//TODO: phasenabschnitt | ZC(on)  -- (off) -- | ZC(on)  ...


void zc_start(void);
void zc_set(t_channel_nr chan_nr, t_action zc_action, uint8_t percent);

#endif /* __ZERO_CROSS_H__ */
