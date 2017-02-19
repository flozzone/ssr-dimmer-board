//
// Created by floz on 18/02/17.
//

#ifndef SSRBOARD_FRAC_TIMER_H
#define SSRBOARD_FRAC_TIMER_H

#include "stdint.h"
#include "zero_cross.h"

#define FRAC_COUNTER_MIN  0
#define FRAC_COUNTER_MAX  255

typedef void (* cb_frac_tick) (uint8_t frac_counter);

void frac_init(cb_frac_tick cb);
void frac_restart(uint8_t width);


#endif //SSRBOARD_FRAC_TIMER_H
