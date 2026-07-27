#ifndef UI_H
#define UI_H

#include <pebble.h>
#include "task_store.h"

void ui_init(void);
void ui_deinit(void);
Window *ui_get_window(void);
void ui_refresh(void);
int ui_get_current_index(void);
void ui_reset_to_first_incomplete(void);
void ui_complete_current_task(void);
GColor ui_category_color(TaskCategory category);
void ui_draw_category_icon(GContext *ctx, GPoint center, TaskCategory category, GColor color);

#endif
