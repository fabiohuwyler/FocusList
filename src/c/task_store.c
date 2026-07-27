#include "task_store.h"
#include <string.h>

static Task s_tasks[MAX_TASKS];
static int s_count = 0;

static void copy_string(char *dest, size_t dest_size, const char *src) {
  if (!src) {
    dest[0] = '\0';
    return;
  }
  strncpy(dest, src, dest_size - 1);
  dest[dest_size - 1] = '\0';
}

void task_store_init(void) {
  task_store_clear();
}

void task_store_deinit(void) {
  task_store_clear();
}

void task_store_clear(void) {
  memset(s_tasks, 0, sizeof(s_tasks));
  s_count = 0;
}

int task_store_count(void) {
  return s_count;
}

void task_store_set_count(int count) {
  if (count < 0) {
    count = 0;
  } else if (count > MAX_TASKS) {
    count = MAX_TASKS;
  }
  s_count = count;
}

const Task *task_store_get(int index) {
  if (index < 0 || index >= s_count) {
    return NULL;
  }
  return &s_tasks[index];
}

void task_store_set_task(int index, const char *id, const char *title, const char *list, const char *due, bool completed, bool overdue) {
  if (index < 0 || index >= MAX_TASKS) {
    return;
  }
  copy_string(s_tasks[index].id, sizeof(s_tasks[index].id), id);
  copy_string(s_tasks[index].title, sizeof(s_tasks[index].title), title);
  copy_string(s_tasks[index].list, sizeof(s_tasks[index].list), list);
  copy_string(s_tasks[index].due, sizeof(s_tasks[index].due), due);
  s_tasks[index].completed = completed;
  s_tasks[index].overdue = overdue;
  s_tasks[index].category = task_store_category_for_list(list);
}

void task_store_set_task_notes(int index, const char *notes) {
  if (index < 0 || index >= MAX_TASKS) {
    return;
  }
  copy_string(s_tasks[index].notes, sizeof(s_tasks[index].notes), notes);
}

void task_store_set_completed(int index, bool completed) {
  if (index < 0 || index >= s_count) {
    return;
  }
  s_tasks[index].completed = completed;
}

int task_store_completed_count(void) {
  int count = 0;
  for (int i = 0; i < s_count; ++i) {
    if (s_tasks[i].completed) {
      ++count;
    }
  }
  return count;
}

bool task_store_all_completed(void) {
  return s_count == 0 || task_store_completed_count() == s_count;
}

int task_store_next_incomplete(int from, int direction) {
  if (s_count == 0) {
    return from;
  }
  int index = from;
  for (int i = 0; i < s_count; ++i) {
    index += direction;
    if (index < 0) {
      index = s_count - 1;
    } else if (index >= s_count) {
      index = 0;
    }
    if (!s_tasks[index].completed) {
      return index;
    }
  }
  return from;
}

TaskCategory task_store_category_for_list(const char *list) {
  if (!list) {
    return TASK_CATEGORY_UNKNOWN;
  }
  if (strcmp(list, "Creative") == 0) return TASK_CATEGORY_CREATIVE;
  if (strcmp(list, "Work") == 0) return TASK_CATEGORY_WORK;
  if (strcmp(list, "Personal") == 0) return TASK_CATEGORY_PERSONAL;
  if (strcmp(list, "Development") == 0) return TASK_CATEGORY_DEVELOPMENT;
  if (strcmp(list, "Wellbeing") == 0) return TASK_CATEGORY_WELLBEING;
  if (strcmp(list, "Home") == 0) return TASK_CATEGORY_HOME;
  return TASK_CATEGORY_UNKNOWN;
}
