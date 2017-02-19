//
// Created by floz on 18/02/17.
//

#include "channels.h"

#include <avr/io.h>

/**
 * output ports:
 *  - digital pin 4: PD4
 *  - digital pin 5: PD5
 *  - digital pin 6: PD6
 *  - digital pin 7: PD7
 *  - digital pin 8: PB0
 */
static t_channel channels[CHANNEL_COUNT] = {
        {
                .id = CHANNEL1,
                .enabled = false,
                .port = &PORTD,
                .ddr = &DDRD,
                .pin = PD4,
        },
        {
                .id = CHANNEL2,
                .enabled = false,
                .port = &PORTD,
                .ddr = &DDRD,
                .pin = PD5,
        },
        {
                .id = CHANNEL3,
                .enabled = false,
                .port = &PORTD,
                .ddr = &DDRD,
                .pin = PD6,
        },
        {
                .id = CHANNEL4,
                .enabled = false,
                .port = &PORTD,
                .ddr = &DDRD,
                .pin = PD7,
        },
        {
                .id = CHANNEL5,
                .enabled = false,
                .port = &PORTB,
                .ddr = &DDRB,
                .pin = PB0,
        }
};

void channels_init() {
  for (int i = 0; i < CHANNEL_COUNT; i++) {
    *channels[i].port &= ~(1<<channels[i].pin);
    *channels[i].ddr |= (1<<channels[i].pin);
  }
}

void channel_set_output(t_channel_nr channel, t_action action) {
  if (action == ON) {
    *channels[channel].port |= (1<<channels[channel].pin);
  } else {
    *channels[channel].port &= ~(1<<channels[channel].pin);
  }
}


// unused
void channel_enable(t_channel_nr chan_nr, t_action action, t_ticks ticks) {

  channels[chan_nr].enabled = false;
  channels[chan_nr].zc_action = action;
  channels[chan_nr].value = ticks;
  channels[chan_nr].enabled = true;
}

void channel_disable(t_channel_nr chan_nr) {
  channels[chan_nr].enabled = false;

  channel_set_output(chan_nr, OFF);
}

t_channel *channel_get(t_channel_nr chan_nr) {
  return &channels[chan_nr];
}
