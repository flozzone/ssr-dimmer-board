#ifndef __ZERO_CROSS_H__
#define __ZERO_CROSS_H__

#include <inttypes.h>

#ifndef F_CPU
# error "F_CPU required to calculate timings."
#endif

// starts the timer at the given value when a rising edge zero cross
// is detected. This is used to compensate the slow zero cross circuit.
#define TIMER_OFFSET_NS               850000
#define CHANNEL_COUNT                 5

enum t_switch {
  OFF,
  ON
};

enum t_wave_type {
  INITIALIZING,
  CALIBRATING,
  POSITIVE,
  NEGATIVE
};

enum t_edge_type{
  FALLING_EDGE,
  RISING_EDGE
};

//TODO: phasenanschnitt | ZC(off) -- (on)  -- | ZC(off) ...
//TODO: phasenabschnitt | ZC(on)  -- (off) -- | ZC(on)  ...


void zc_init(void);
void zc_set_channel(uint8_t chan_number, uint8_t zc_action, uint8_t frac);

#endif /* __ZERO_CROSS_H__ */
