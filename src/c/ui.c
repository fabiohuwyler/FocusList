#include "ui.h"
#include "theme.h"
#include "animation.h"
#include "detail_view.h"
#include <string.h>

#define COMPLETION_DELAY_MS 850

static Window *s_main_window = NULL;
static Window *s_completion_window = NULL;
static Layer *s_main_layer = NULL;
static Layer *s_completion_layer = NULL;
static AppTimer *s_completion_timer = NULL;

static char s_time_buffer[16];
static int s_current_index = 0;
static bool s_focus_locked = false;

static void update_time(void) {
  time_t now = time(NULL);
  struct tm *time_info = localtime(&now);

  if (clock_is_24h_style()) {
    strftime(s_time_buffer, sizeof(s_time_buffer), "%H:%M", time_info);
  } else {
    strftime(s_time_buffer, sizeof(s_time_buffer), "%I:%M", time_info);
    if (s_time_buffer[0] == '0') {
      memmove(s_time_buffer, s_time_buffer + 1, strlen(s_time_buffer));
    }
  }
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
  if (s_main_layer) {
    layer_mark_dirty(s_main_layer);
  }
}

GColor ui_category_color(TaskCategory category) {
#ifdef PBL_COLOR
  switch (category) {
    case TASK_CATEGORY_CREATIVE: return GColorFromHEX(0x7B5A8E);
    case TASK_CATEGORY_WORK: return GColorFromHEX(0x4B6E8A);
    case TASK_CATEGORY_PERSONAL: return GColorFromHEX(0xA06B37);
    case TASK_CATEGORY_DEVELOPMENT: return GColorFromHEX(0x555555);
    case TASK_CATEGORY_WELLBEING: return GColorFromHEX(0x4E7A5A);
    case TASK_CATEGORY_HOME: return GColorFromHEX(0x7A6A42);
    case TASK_CATEGORY_UNKNOWN: return GColorFromHEX(0x77705F);
  }
#endif
  return GColorBlack;
}

void ui_draw_category_icon(GContext *ctx, GPoint center, TaskCategory category, GColor color) {
  graphics_context_set_stroke_color(ctx, color);
  graphics_context_set_fill_color(ctx, color);

  switch (category) {
    case TASK_CATEGORY_CREATIVE:
      graphics_draw_line(ctx, GPoint(center.x - 5, center.y + 5), GPoint(center.x + 4, center.y - 4));
      graphics_draw_line(ctx, GPoint(center.x - 3, center.y + 6), GPoint(center.x + 6, center.y - 3));
      graphics_fill_circle(ctx, GPoint(center.x + 5, center.y - 4), 2);
      break;
    case TASK_CATEGORY_WORK:
      graphics_draw_rect(ctx, GRect(center.x - 6, center.y - 4, 12, 9));
      graphics_draw_line(ctx, GPoint(center.x - 3, center.y - 6), GPoint(center.x + 3, center.y - 6));
      graphics_draw_line(ctx, GPoint(center.x - 3, center.y - 6), GPoint(center.x - 3, center.y - 4));
      graphics_draw_line(ctx, GPoint(center.x + 3, center.y - 6), GPoint(center.x + 3, center.y - 4));
      break;
    case TASK_CATEGORY_PERSONAL:
      graphics_draw_circle(ctx, GPoint(center.x, center.y - 3), 3);
      graphics_draw_arc(ctx, GRect(center.x - 6, center.y, 12, 10), GOvalScaleModeFitCircle, DEG_TO_TRIGANGLE(200), DEG_TO_TRIGANGLE(340));
      break;
    case TASK_CATEGORY_DEVELOPMENT:
      graphics_draw_rect(ctx, GRect(center.x - 6, center.y - 5, 12, 9));
      graphics_draw_line(ctx, GPoint(center.x - 3, center.y + 6), GPoint(center.x + 3, center.y + 6));
      graphics_draw_line(ctx, GPoint(center.x, center.y + 4), GPoint(center.x, center.y + 6));
      break;
    case TASK_CATEGORY_WELLBEING:
      graphics_draw_line(ctx, GPoint(center.x, center.y + 6), GPoint(center.x, center.y - 5));
      graphics_draw_line(ctx, GPoint(center.x, center.y - 1), GPoint(center.x - 5, center.y - 4));
      graphics_draw_line(ctx, GPoint(center.x, center.y + 1), GPoint(center.x + 5, center.y - 2));
      break;
    case TASK_CATEGORY_HOME:
    case TASK_CATEGORY_UNKNOWN:
    default:
      graphics_draw_line(ctx, GPoint(center.x - 6, center.y), GPoint(center.x, center.y - 6));
      graphics_draw_line(ctx, GPoint(center.x, center.y - 6), GPoint(center.x + 6, center.y));
      graphics_draw_rect(ctx, GRect(center.x - 5, center.y, 10, 7));
      break;
  }
}

static void draw_progress_dots(GContext *ctx, GRect bounds) {
  const int count = task_store_count();
  if (count <= 0) {
    return;
  }

  const int spacing = PBL_IF_ROUND_ELSE(16, 15);
  const int total_width = (count - 1) * spacing;
  const int start_x = bounds.size.w / 2 - total_width / 2;
  const int y = PBL_IF_ROUND_ELSE(bounds.size.h - 43, bounds.size.h - 39);
  const int completed = task_store_completed_count();

  for (int i = 0; i < count; ++i) {
    GPoint point = GPoint(start_x + i * spacing, y);
    graphics_context_set_stroke_color(ctx, COLOR_ACCENT);
    if (i < completed) {
      graphics_context_set_fill_color(ctx, COLOR_COMPLETE);
      graphics_fill_circle(ctx, point, 4);
    } else {
      graphics_draw_circle(ctx, point, 4);
    }
  }
}

static void draw_empty_state(GContext *ctx, GRect bounds) {
  graphics_context_set_text_color(ctx, COLOR_ACCENT);
  graphics_draw_text(
    ctx,
    "Everything\nis done.",
    fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD),
    GRect(18, PBL_IF_ROUND_ELSE(56, 48), bounds.size.w - 36, 74),
    GTextOverflowModeWordWrap,
    GTextAlignmentCenter,
    NULL
  );

  graphics_context_set_text_color(ctx, COLOR_MUTED);
  graphics_draw_text(
    ctx,
    "Enjoy the rest\nof your day.",
    fonts_get_system_font(FONT_KEY_GOTHIC_18),
    GRect(20, PBL_IF_ROUND_ELSE(128, 119), bounds.size.w - 40, 50),
    GTextOverflowModeWordWrap,
    GTextAlignmentCenter,
    NULL
  );
}

static void main_layer_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  graphics_context_set_fill_color(ctx, COLOR_BG);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);

  update_time();

  graphics_context_set_text_color(ctx, COLOR_TEXT);
  graphics_draw_text(
    ctx,
    s_time_buffer,
    fonts_get_system_font(PBL_IF_ROUND_ELSE(FONT_KEY_LECO_32_BOLD_NUMBERS, FONT_KEY_LECO_28_LIGHT_NUMBERS)),
    GRect(0, PBL_IF_ROUND_ELSE(12, 6), bounds.size.w, 38),
    GTextOverflowModeFill,
    GTextAlignmentCenter,
    NULL
  );

  if (task_store_all_completed()) {
    draw_empty_state(ctx, bounds);
    return;
  }

  const Task *task = task_store_get(s_current_index);
  if (!task) {
    return;
  }

  const int offset = animation_get_offset();

  graphics_context_set_text_color(ctx, task->overdue ? COLOR_OVERDUE : COLOR_ACCENT);
  graphics_draw_text(
    ctx,
    s_focus_locked ? "Focus" : "Today",
    fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD),
    GRect(offset, PBL_IF_ROUND_ELSE(52, 43), bounds.size.w, 24),
    GTextOverflowModeFill,
    GTextAlignmentCenter,
    NULL
  );

  graphics_context_set_text_color(ctx, COLOR_TEXT);
  graphics_draw_text(
    ctx,
    task->title,
    fonts_get_system_font(PBL_IF_ROUND_ELSE(FONT_KEY_GOTHIC_24_BOLD, FONT_KEY_GOTHIC_24_BOLD)),
    GRect(14 + offset, PBL_IF_ROUND_ELSE(77, 66), bounds.size.w - 28, PBL_IF_ROUND_ELSE(66, 58)),
    GTextOverflowModeWordWrap,
    GTextAlignmentCenter,
    NULL
  );

  const int meta_y = PBL_IF_ROUND_ELSE(143, 125);
  GColor cat_color = ui_category_color(task->category);
  ui_draw_category_icon(ctx, GPoint(bounds.size.w / 2 - 34 + offset, meta_y + 8), task->category, cat_color);

  graphics_context_set_text_color(ctx, cat_color);
  graphics_draw_text(
    ctx,
    task->list,
    fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD),
    GRect(bounds.size.w / 2 - 24 + offset, meta_y - 3, bounds.size.w / 2 + 10, 24),
    GTextOverflowModeFill,
    GTextAlignmentLeft,
    NULL
  );

  graphics_context_set_text_color(ctx, task->overdue ? COLOR_OVERDUE : COLOR_MUTED);
  graphics_draw_text(
    ctx,
    task->due,
    fonts_get_system_font(FONT_KEY_GOTHIC_18),
    GRect(offset, meta_y + 22, bounds.size.w, 24),
    GTextOverflowModeFill,
    GTextAlignmentCenter,
    NULL
  );

  draw_progress_dots(ctx, bounds);

  const int line_y = bounds.size.h - 24;
  graphics_context_set_stroke_color(ctx, COLOR_LINE);
  graphics_draw_line(ctx, GPoint(20, line_y), GPoint(bounds.size.w - 20, line_y));

  graphics_context_set_text_color(ctx, COLOR_MUTED);
  graphics_draw_text(
    ctx,
    "Select · Details",
    fonts_get_system_font(FONT_KEY_GOTHIC_14),
    GRect(0, line_y + 2, bounds.size.w, 18),
    GTextOverflowModeFill,
    GTextAlignmentCenter,
    NULL
  );
}

static void show_next_task(int direction) {
  if (task_store_all_completed() || s_focus_locked) {
    return;
  }

  const int next = task_store_next_incomplete(s_current_index, direction);
  if (next == s_current_index) {
    return;
  }

  s_current_index = next;
  animation_start(direction);
}

static void close_completion(void *context) {
  s_completion_timer = NULL;
  if (window_stack_contains_window(s_completion_window)) {
    window_stack_remove(s_completion_window, true);
  }
  ui_refresh();
}

void ui_complete_current_task(void) {
  if (task_store_all_completed()) {
    return;
  }

  task_store_set_completed(s_current_index, true);
  s_focus_locked = false;
  vibes_short_pulse();

  if (!task_store_all_completed()) {
    s_current_index = task_store_next_incomplete(s_current_index, 1);
  }

  animation_stop();
  ui_refresh();

  window_stack_push(s_completion_window, true);

  if (s_completion_timer) {
    app_timer_cancel(s_completion_timer);
  }
  s_completion_timer = app_timer_register(COMPLETION_DELAY_MS, close_completion, NULL);
}

static void main_up_handler(ClickRecognizerRef recognizer, void *context) {
  show_next_task(-1);
}

static void main_down_handler(ClickRecognizerRef recognizer, void *context) {
  show_next_task(1);
}

static void main_select_handler(ClickRecognizerRef recognizer, void *context) {
  if (task_store_all_completed()) {
    return;
  }
  detail_view_update(s_current_index);
  window_stack_push(detail_view_get_window(), true);
}

static void main_long_select_handler(ClickRecognizerRef recognizer, void *context) {
  ui_complete_current_task();
}

static void main_long_up_handler(ClickRecognizerRef recognizer, void *context) {
  if (task_store_all_completed()) {
    return;
  }
  s_focus_locked = !s_focus_locked;
  if (s_focus_locked) {
    vibes_short_pulse();
  } else {
    vibes_double_pulse();
  }
  ui_refresh();
}

static void main_click_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_UP, main_up_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, main_down_handler);
  window_single_click_subscribe(BUTTON_ID_SELECT, main_select_handler);
  window_long_click_subscribe(BUTTON_ID_SELECT, 600, main_long_select_handler, NULL);
  window_long_click_subscribe(BUTTON_ID_UP, 700, main_long_up_handler, NULL);
}

static void completion_layer_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  graphics_context_set_fill_color(ctx, COLOR_BG);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);

  graphics_context_set_stroke_color(ctx, COLOR_COMPLETE);
  graphics_context_set_stroke_width(ctx, 4);
  GPoint center = GPoint(bounds.size.w / 2, PBL_IF_ROUND_ELSE(63, 52));
  graphics_draw_circle(ctx, center, 20);
  graphics_draw_line(ctx, GPoint(center.x - 9, center.y), GPoint(center.x - 2, center.y + 7));
  graphics_draw_line(ctx, GPoint(center.x - 2, center.y + 7), GPoint(center.x + 11, center.y - 8));
  graphics_context_set_stroke_width(ctx, 1);

  graphics_context_set_text_color(ctx, COLOR_TEXT);
  graphics_draw_text(
    ctx,
    "Nicely done.",
    fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD),
    GRect(8, PBL_IF_ROUND_ELSE(93, 82), bounds.size.w - 16, 34),
    GTextOverflowModeFill,
    GTextAlignmentCenter,
    NULL
  );

  graphics_context_set_text_color(ctx, COLOR_MUTED);
  graphics_draw_text(
    ctx,
    task_store_all_completed() ? "Everything is complete." : "One thing less.",
    fonts_get_system_font(FONT_KEY_GOTHIC_18),
    GRect(12, PBL_IF_ROUND_ELSE(126, 116), bounds.size.w - 24, 42),
    GTextOverflowModeWordWrap,
    GTextAlignmentCenter,
    NULL
  );
}

static void main_window_load(Window *window) {
  Layer *root = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(root);

  window_set_background_color(window, COLOR_BG);
  window_set_click_config_provider(window, main_click_provider);

  s_main_layer = layer_create(bounds);
  layer_set_update_proc(s_main_layer, main_layer_update_proc);
  layer_add_child(root, s_main_layer);

  animation_init(s_main_layer);
}

static void main_window_unload(Window *window) {
  if (s_main_layer) {
    layer_destroy(s_main_layer);
    s_main_layer = NULL;
  }
}

static void completion_window_load(Window *window) {
  Layer *root = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(root);
  window_set_background_color(window, COLOR_BG);

  s_completion_layer = layer_create(bounds);
  layer_set_update_proc(s_completion_layer, completion_layer_update_proc);
  layer_add_child(root, s_completion_layer);
}

static void completion_window_unload(Window *window) {
  if (s_completion_layer) {
    layer_destroy(s_completion_layer);
    s_completion_layer = NULL;
  }
}

void ui_init(void) {
  update_time();

  s_main_window = window_create();
  s_completion_window = window_create();

  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  window_set_window_handlers(s_completion_window, (WindowHandlers) {
    .load = completion_window_load,
    .unload = completion_window_unload
  });

  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  window_stack_push(s_main_window, true);
}

void ui_deinit(void) {
  tick_timer_service_unsubscribe();

  if (s_completion_timer) {
    app_timer_cancel(s_completion_timer);
    s_completion_timer = NULL;
  }

  animation_stop();

  if (s_completion_window) {
    window_destroy(s_completion_window);
    s_completion_window = NULL;
  }
  if (s_main_window) {
    window_destroy(s_main_window);
    s_main_window = NULL;
  }
}

Window *ui_get_window(void) {
  return s_main_window;
}

void ui_refresh(void) {
  if (s_main_layer) {
    layer_mark_dirty(s_main_layer);
  }
}

int ui_get_current_index(void) {
  return s_current_index;
}

void ui_reset_to_first_incomplete(void) {
  if (task_store_all_completed()) {
    s_current_index = 0;
  } else {
    s_current_index = task_store_next_incomplete(-1, 1);
    if (s_current_index < 0) {
      s_current_index = 0;
    }
  }
  ui_refresh();
}
