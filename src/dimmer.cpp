#include "dimmer.h"
#include "debug.h"
#include "zero_cross.h"
#include "list.h"
#include "channels.h"


static volatile uint16_t frac_width = 0;
static struct list event_list;
volatile struct list_elem *current_event;
volatile t_channel *current_channel;

extern uint8_t burst_lut[256][2];

static void zero_cross_callback(t_edge edge, t_ticks wave_width);
static void frac_tick_callback(uint8_t frac);
static void eval_frac(uint8_t frac);
static void eval_burst(t_channel *chan);
static bool event_less_fct (const struct list_elem *a,
                     const struct list_elem *b,
                     void *aux);
static inline void unschedule_event(t_channel *chan);
static inline void schedule_event(t_channel *chan);


/**
 * Zero cross callback function running in interrupt.
 * @param edge
 */
static void zero_cross_callback(t_edge edge, t_ticks wave_width) {
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
        switch (chan->fire_type) {
          case PHASE_LEADING_EDGE:
          case PHASE_TRAILING_EDGE: {
            channel_set_output(i, INVERT_ACTION(chan->zc_action));
            break;
          }
          case FULL_WAVE_BURST: {
            if (edge != RISING_EDGE)
              break;
          }
          case HALF_WAVE_BURST: {
            eval_burst(chan);
          }
        }
      }
    }
  }
}

static void frac_tick_callback(uint8_t frac) {
  debug_toggleB();

  eval_frac(frac);
}

static void eval_burst(t_channel *chan) {
  if (chan->current_bursts == 0) {
    uint8_t next_burst = burst_lut[chan->value][!chan->current_action];
    if (next_burst != 0) {
      // -1 because we loose one cycle in this logic
      chan->current_bursts = next_burst - 1;
      chan->current_action = !chan->current_action;
      channel_set_output(chan->id, chan->current_action);
    } else {
      chan->current_bursts = burst_lut[chan->value][chan->current_action];
    }
  } else {
    chan->current_bursts--;
  }
}

static inline void next_channel (void) {
  current_event = list_next((struct list_elem *)current_event);
  current_channel = list_entry (current_event, t_channel, elem);
}

static void eval_frac(uint8_t frac) {
  while (current_channel->value == frac) {
    channel_set_output(current_channel->id, current_channel->zc_action);

    next_channel();
  }
}

static bool event_less_fct (const struct list_elem *a,
                            const struct list_elem *b,
                            void *aux) {

  t_channel *eventA = list_entry (a, t_channel, elem);
  t_channel *eventB = list_entry (b, t_channel, elem);

  return (eventA->value < eventB->value) ? true : false;
}

static inline void unschedule_event(t_channel *chan) {

  // handle next channels frac number if this is to be removed
  if (current_channel == chan) {
    next_channel();
  }

  if (chan->elem.next != NULL) {
    // remove from the event list and set pointers to NULL
    // NULL pointers will be used to determine if a channel
    // is in the event list or not
    list_remove(&chan->elem);
    chan->elem.next = NULL;
    chan->elem.prev = NULL;
  }
}

static inline void schedule_event(t_channel *chan) {
  if (chan->elem.next != NULL) {
    unschedule_event(chan);
  }

  list_insert_ordered(&event_list, &chan->elem, &event_less_fct, NULL);
}

void dimmer_init(void) {
  debug_init();

  list_init(&event_list);

  channels_init();

  zc_start(&zero_cross_callback);
  frac_init(&frac_tick_callback);
}

void dimmer_set(t_fire_type fire_type, t_channel_nr chan_nr, uint8_t value) {

  t_channel *chan = channel_get(chan_nr);

  chan->enabled = false;
  chan->value = value;
  chan->fire_type = fire_type;

  switch (fire_type) {
    case PHASE_TRAILING_EDGE: {
      chan->zc_action = ON;
      schedule_event(chan);
      break;
    }
    case PHASE_LEADING_EDGE: {
      chan->zc_action = OFF;
      schedule_event(chan);
      break;
    }
    case FULL_WAVE_BURST: {
      break;
    }
    case HALF_WAVE_BURST: {
      break;
    }
    default: {
      // should never be reached
      return;
    }
  }

  chan->enabled = true;
}

void dimmer_disable(t_channel_nr chan_nr) {
  if (chan_nr < CHANNEL_COUNT) {
    channel_disable(chan_nr);
    unschedule_event(channel_get(chan_nr));
  }
}

