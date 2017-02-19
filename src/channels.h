//
// Created by floz on 18/02/17.
//

#ifndef SSRBOARD_CHANNELS_H
#define SSRBOARD_CHANNELS_H

#include <stdint.h>

#include "zero_cross.h"
#include "list.h"

#define CHANNEL_COUNT           5

typedef uint8_t t_channel_nr;
typedef bool t_action;

typedef struct {
    t_channel_nr id;
    bool enabled;
    volatile uint8_t *port;
    volatile uint8_t *ddr;
    volatile uint8_t pin;
    t_action zc_action;
    t_ticks value;

    struct list_elem elem;
} t_channel;

enum e_channels {
    CHANNEL1 = 0,
    CHANNEL2,
    CHANNEL3,
    CHANNEL4,
    CHANNEL5
};

enum e_action {
    OFF = 0,
    ON = 1
};

#define INVERT_ACTION(action) !action

void channels_init();
void channel_set_output(t_channel_nr channel, t_action action);
void channel_enable(t_channel_nr chan_nr, t_action action, t_ticks ticks);
void channel_disable(t_channel_nr chan_nr);
t_channel *channel_get(t_channel_nr chan_nr);

#endif //SSRBOARD_CHANNELS_H
