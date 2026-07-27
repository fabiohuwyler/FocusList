#ifndef ANIMATION_H
#define ANIMATION_H

#include <pebble.h>

void animation_init(Layer *main_layer);
void animation_start(int direction);
void animation_stop(void);
int animation_get_offset(void);

#endif
