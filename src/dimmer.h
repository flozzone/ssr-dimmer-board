#ifndef __DIMMER_H__
#define __DIMMER_H__

#include <inttypes.h>

#include "zero_cross.h"
#include "frac.h"
#include "channels.h"

void dimmer_init(void);
void dimmer_set(t_channel_nr chan_nr, t_action zc_action, uint8_t value);
void dimmer_disable(t_channel_nr chan_nr);


#endif /* __DIMMER_H__ */