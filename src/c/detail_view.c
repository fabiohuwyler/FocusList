#include "detail_view.h"
#include "ui.h"
#include "theme.h"
#include "task_store.h"

static Window *s_details_window = NULL;
static ScrollLayer *s_details_scroll = NULL;
static Layer *s_details_header_layer = NULL;
static TextLayer *s_details_title_layer = NULL;
static TextLayer *s_details_meta_layer = NULL;
static TextLayer *s_details_notes_layer = NULL;
static TextLayer *s_details_hint_layer = NULL;

static int s_current_detail_index = 0;

static TextLayer *create_text_layer(Layer *parent, GRect frame, GFont font, GTextAlignment alignment, GColor color) {
  TextLayer *layer = text_layer_create(frame);
  text_layer_set_background_color(layer, GColorClear);
  text_layer_set_text_color(layer, color);
  text_layer_set_font(layer, font);
  text_layer_set_text_alignment(layer, alignment);
  text_layer_set_overflow_mode(layer, GTextOverflowModeWordWrap);
  layer_add_child(parent, text_layer_get_layer(layer));
  return layer;
}

static void details_header_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  const Task *task = task_store_get(s_current_detail_index);
  graphics_context_set_fill_color(ctx, COLOR_BG);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);
  if (task) {
    ui_draw_category_icon(ctx, GPoint(18, bounds.size.h / 2), task->category, ui_category_color(task->category));
  }
}

static void update_detail_layers(int task_index) {
  s_current_detail_index = task_index;
  const Task *task = task_store_get(task_index);
  if (!task) {
    return;
  }

  text_layer_set_text(s_details_title_layer, task->title);

  static char meta[80];
  snprintf(meta, sizeof(meta), "%s\n%s", task->list, task->due);
  text_layer_set_text(s_details_meta_layer, meta);
  text_layer_set_text_color(s_details_meta_layer, task->overdue ? COLOR_OVERDUE : ui_category_color(task->category));

  text_layer_set_text(s_details_notes_layer, task->notes);
  text_layer_set_text(s_details_hint_layer, "Hold Select to complete");

  if (s_details_scroll) {
    scroll_layer_set_content_offset(s_details_scroll, GPointZero, false);
  }
  if (s_details_header_layer) {
    layer_mark_dirty(s_details_header_layer);
  }
}

static void details_long_select_handler(ClickRecognizerRef recognizer, void *context) {
  window_stack_pop(true);
  ui_complete_current_task();
}

static void details_click_provider(void *context) {
  scroll_layer_set_click_config_onto_window(s_details_scroll, s_details_window);
  window_long_click_subscribe(BUTTON_ID_SELECT, 600, details_long_select_handler, NULL);
}

static void details_window_load(Window *window) {
  Layer *root = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(root);
  window_set_background_color(window, COLOR_BG);

  s_details_scroll = scroll_layer_create(bounds);
  scroll_layer_set_shadow_hidden(s_details_scroll, true);
  scroll_layer_set_content_size(s_details_scroll, GSize(bounds.size.w, PBL_IF_ROUND_ELSE(360, 330)));
  layer_add_child(root, scroll_layer_get_layer(s_details_scroll));

  Layer *content = scroll_layer_get_layer(s_details_scroll);

  s_details_header_layer = layer_create(GRect(0, 0, bounds.size.w, 38));
  layer_set_update_proc(s_details_header_layer, details_header_update_proc);
  scroll_layer_add_child(s_details_scroll, s_details_header_layer);

  s_details_title_layer = create_text_layer(
    content,
    GRect(PBL_IF_ROUND_ELSE(20, 12), 38, bounds.size.w - PBL_IF_ROUND_ELSE(40, 24), 72),
    fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD),
    GTextAlignmentCenter,
    COLOR_TEXT
  );

  s_details_meta_layer = create_text_layer(
    content,
    GRect(PBL_IF_ROUND_ELSE(22, 14), 112, bounds.size.w - PBL_IF_ROUND_ELSE(44, 28), 48),
    fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD),
    GTextAlignmentCenter,
    COLOR_ACCENT
  );

  s_details_notes_layer = create_text_layer(
    content,
    GRect(PBL_IF_ROUND_ELSE(24, 16), 170, bounds.size.w - PBL_IF_ROUND_ELSE(48, 32), 120),
    fonts_get_system_font(FONT_KEY_GOTHIC_18),
    GTextAlignmentLeft,
    COLOR_TEXT
  );

  s_details_hint_layer = create_text_layer(
    content,
    GRect(8, PBL_IF_ROUND_ELSE(316, 292), bounds.size.w - 16, 24),
    fonts_get_system_font(FONT_KEY_GOTHIC_14),
    GTextAlignmentCenter,
    COLOR_MUTED
  );

#ifdef PBL_ROUND
  text_layer_enable_screen_text_flow_and_paging(s_details_title_layer, 4);
  text_layer_enable_screen_text_flow_and_paging(s_details_notes_layer, 4);
#endif

  window_set_click_config_provider(window, details_click_provider);
  update_detail_layers(s_current_detail_index);
}

static void details_window_unload(Window *window) {
  if (s_details_hint_layer) {
    text_layer_destroy(s_details_hint_layer);
    s_details_hint_layer = NULL;
  }
  if (s_details_notes_layer) {
    text_layer_destroy(s_details_notes_layer);
    s_details_notes_layer = NULL;
  }
  if (s_details_meta_layer) {
    text_layer_destroy(s_details_meta_layer);
    s_details_meta_layer = NULL;
  }
  if (s_details_title_layer) {
    text_layer_destroy(s_details_title_layer);
    s_details_title_layer = NULL;
  }
  if (s_details_header_layer) {
    layer_destroy(s_details_header_layer);
    s_details_header_layer = NULL;
  }
  if (s_details_scroll) {
    scroll_layer_destroy(s_details_scroll);
    s_details_scroll = NULL;
  }
}

void detail_view_init(void) {
  s_details_window = window_create();
  window_set_window_handlers(s_details_window, (WindowHandlers) {
    .load = details_window_load,
    .unload = details_window_unload
  });
}

void detail_view_deinit(void) {
  if (s_details_window) {
    window_destroy(s_details_window);
    s_details_window = NULL;
  }
}

Window *detail_view_get_window(void) {
  return s_details_window;
}

void detail_view_update(int task_index) {
  s_current_detail_index = task_index;
  if (s_details_title_layer) {
    update_detail_layers(task_index);
  }
}
