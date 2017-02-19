#include "dimmer.h"
#include "debug.h"
#include "zero_cross.h"
#include "list.h"
#include "channels.h"


static volatile uint16_t frac_width = 0;
static struct list event_list;
volatile struct list_elem *current_event;
volatile t_channel *current_channel;

static void eval(uint8_t frac);
bool event_less_fct (const struct list_elem *a,
                     const struct list_elem *b,
                     void *aux);

/**
 * Zero cross callback function running in interrupt.
 * @param edge
 */
void zero_cross_callback(t_edge edge, t_ticks wave_width) {
  debug_toggleA();

  if (edge == RISING_EDGE) {
    frac_width = wave_width >> 9;
  }

  if (frac_width > 0) {
    frac_restart(frac_width);

    current_event = list_begin(&event_list);
    current_channel = list_entry (current_event, t_channel, elem);

    for (uint8_t i = 0; i < CHANNEL_COUNT; i++) {
      t_channel *chan = channel_get(i);
      if (chan->enabled) {
        channel_set_output(i, INVERT_ACTION(chan->zc_action));
      }
    }
  }
}

void frac_tick_callback(uint8_t frac) {
  debug_toggleB();

  eval(frac);
}

static void eval(uint8_t frac) {
  while (current_channel->value == frac) {
    channel_set_output(current_channel->id, current_channel->zc_action);

    current_event = list_next((struct list_elem *)current_event);
    current_channel = list_entry (current_event, t_channel, elem);
  }
}

void dimmer_init(void) {
  debug_init();

  list_init(&event_list);

  debug_toggleA();
  channels_init();
  zc_start(&zero_cross_callback);
  frac_init(&frac_tick_callback);
}

void dimmer_set(t_channel_nr chan_nr, t_action zc_action, uint8_t value) {
  if (chan_nr < CHANNEL_COUNT) {
    channel_enable(chan_nr, zc_action, value);

    list_insert_ordered(&event_list, &channel_get(chan_nr)->elem, &event_less_fct, NULL);
  }
}

void dimmer_disable(t_channel_nr chan_nr) {
  if (chan_nr < CHANNEL_COUNT) {
    channel_disable(chan_nr);
  }
}

bool event_less_fct (const struct list_elem *a,
                     const struct list_elem *b,
                     void *aux) {

  t_channel *eventA = list_entry (a, t_channel, elem);
  t_channel *eventB = list_entry (b, t_channel, elem);

  return (eventA->value < eventB->value) ? true : false;
}