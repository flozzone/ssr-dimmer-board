#ifndef __ZERO_CROSS_H__
#define __ZERO_CROSS_H__

#include <inttypes.h>

#ifndef F_CPU
# error "F_CPU required to calculate timings."
#endif

// starts the timer at the given value when a rising edge zero cross
// is detected. This is used to compensate the slow zero cross circuit.
#define TIMER_OFFSET_NS               650000

enum t_wave_type_ {
  INITIALIZING,
  POSITIVE,
  NEGATIVE
} t_wave_type;

enum t_edge_type_ {
  FALLING_EDGE,
  RISING_EDGE
} t_edge_type;

//TODO: phasenanschnitt | ZC(off) -- (on)  -- | ZC(off) ...
//TODO: phasenabschnitt | ZC(on)  -- (off) -- | ZC(on)  ...

void zc_init(void);

#endif /* __ZERO_CROSS_H__ */
