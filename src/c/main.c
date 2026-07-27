#include <pebble.h>

#include "task_store.h"
#include "sync.h"
#include "ui.h"
#include "detail_view.h"

int main(void) {
  task_store_init();
  sync_init();
  ui_init();
  detail_view_init();

  sync_request();

  app_event_loop();

  detail_view_deinit();
  ui_deinit();
  sync_deinit();
  task_store_deinit();

  return 0;
}
