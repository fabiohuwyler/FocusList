#include "sync.h"
#include "task_store.h"
#include "ui.h"

#define MESSAGE_TYPE_REQUEST 0
#define MESSAGE_TYPE_TASK_START 1
#define MESSAGE_TYPE_TASK 2
#define MESSAGE_TYPE_TASK_NOTES 3
#define MESSAGE_TYPE_TASK_END 4

static void inbox_received_handler(DictionaryIterator *iter, void *context) {
  Tuple *type_t = dict_find(iter, MESSAGE_KEY_MESSAGE_TYPE);
  if (!type_t) {
    return;
  }
  const int32_t type = type_t->value->int32;

  switch (type) {
    case MESSAGE_TYPE_TASK_START: {
      Tuple *count_t = dict_find(iter, MESSAGE_KEY_TASK_COUNT);
      if (count_t) {
        task_store_clear();
        task_store_set_count(count_t->value->int32);
      }
      break;
    }

    case MESSAGE_TYPE_TASK: {
      Tuple *idx_t = dict_find(iter, MESSAGE_KEY_TASK_INDEX);
      Tuple *id_t = dict_find(iter, MESSAGE_KEY_TASK_ID);
      Tuple *title_t = dict_find(iter, MESSAGE_KEY_TASK_TITLE);
      Tuple *list_t = dict_find(iter, MESSAGE_KEY_TASK_LIST);
      Tuple *due_t = dict_find(iter, MESSAGE_KEY_TASK_DUE);
      Tuple *completed_t = dict_find(iter, MESSAGE_KEY_TASK_COMPLETED);
      Tuple *overdue_t = dict_find(iter, MESSAGE_KEY_TASK_OVERDUE);
      if (idx_t) {
        task_store_set_task(
          idx_t->value->int32,
          id_t ? id_t->value->cstring : NULL,
          title_t ? title_t->value->cstring : NULL,
          list_t ? list_t->value->cstring : NULL,
          due_t ? due_t->value->cstring : NULL,
          completed_t ? (completed_t->value->int32 != 0) : false,
          overdue_t ? (overdue_t->value->int32 != 0) : false
        );
      }
      break;
    }

    case MESSAGE_TYPE_TASK_NOTES: {
      Tuple *idx_t = dict_find(iter, MESSAGE_KEY_TASK_INDEX);
      Tuple *notes_t = dict_find(iter, MESSAGE_KEY_TASK_NOTES);
      if (idx_t && notes_t) {
        task_store_set_task_notes(idx_t->value->int32, notes_t->value->cstring);
      }
      break;
    }

    case MESSAGE_TYPE_TASK_END: {
      ui_reset_to_first_incomplete();
      break;
    }

    default:
      break;
  }

  ui_refresh();
}

static void outbox_sent_handler(DictionaryIterator *iter, void *context) {
  (void)iter;
  (void)context;
}

static void outbox_failed_handler(DictionaryIterator *iter, AppMessageResult reason, void *context) {
  (void)iter;
  (void)reason;
  (void)context;
}

static void inbox_dropped_handler(AppMessageResult reason, void *context) {
  (void)reason;
  (void)context;
}

void sync_init(void) {
  app_message_register_inbox_received(inbox_received_handler);
  app_message_register_outbox_sent(outbox_sent_handler);
  app_message_register_outbox_failed(outbox_failed_handler);
  app_message_register_inbox_dropped(inbox_dropped_handler);
  app_message_open(256, 64);
}

void sync_deinit(void) {
  app_message_deregister_callbacks();
}

void sync_request(void) {
  DictionaryIterator *iter;
  AppMessageResult result = app_message_outbox_begin(&iter);
  if (result != APP_MSG_OK) {
    return;
  }
  dict_write_int32(iter, MESSAGE_KEY_REQUEST_TASK, 1);
  dict_write_end(iter);
  app_message_outbox_send();
}
