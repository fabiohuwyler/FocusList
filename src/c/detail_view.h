#ifndef DETAIL_VIEW_H
#define DETAIL_VIEW_H

#include <pebble.h>

void detail_view_init(void);
void detail_view_deinit(void);
Window *detail_view_get_window(void);
void detail_view_update(int task_index);

#endif
