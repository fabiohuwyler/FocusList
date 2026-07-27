#include "animation.h"

#define ANIMATION_STEPS 6
#define ANIMATION_INTERVAL_MS 24

static Layer *s_main_layer = NULL;
static AppTimer *s_timer = NULL;
static int s_step = 0;
static int s_direction = 0;
static int s_offset = 0;

static void animation_tick(void *context) {
  if (!s_main_layer) {
    s_timer = NULL;
    return;
  }

  ++s_step;
  const int width = layer_get_bounds(s_main_layer).size.w;
  const int remaining = ANIMATION_STEPS - s_step;
  s_offset = (s_direction * width * remaining) / ANIMATION_STEPS;

  layer_mark_dirty(s_main_layer);

  if (s_step < ANIMATION_STEPS) {
    s_timer = app_timer_register(ANIMATION_INTERVAL_MS, animation_tick, NULL);
  } else {
    s_offset = 0;
    s_direction = 0;
    s_step = 0;
    s_timer = NULL;
    layer_mark_dirty(s_main_layer);
  }
}

void animation_init(Layer *main_layer) {
  s_main_layer = main_layer;
  s_offset = 0;
  s_step = 0;
  s_direction = 0;
  s_timer = NULL;
}

void animation_start(int direction) {
  animation_stop();
  s_direction = direction;
  s_step = 0;
  s_offset = direction * layer_get_bounds(s_main_layer).size.w;
  layer_mark_dirty(s_main_layer);
  s_timer = app_timer_register(ANIMATION_INTERVAL_MS, animation_tick, NULL);
}

void animation_stop(void) {
  if (s_timer) {
    app_timer_cancel(s_timer);
    s_timer = NULL;
  }
  s_offset = 0;
  s_direction = 0;
  s_step = 0;
  if (s_main_layer) {
    layer_mark_dirty(s_main_layer);
  }
}

int animation_get_offset(void) {
  return s_offset;
}
