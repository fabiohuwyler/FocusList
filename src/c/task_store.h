#ifndef TASK_STORE_H
#define TASK_STORE_H

#include <pebble.h>
#include <stdbool.h>

#define MAX_TASKS 32
#define MAX_ID_LENGTH 33
#define MAX_TITLE_LENGTH 64
#define MAX_LIST_LENGTH 32
#define MAX_DUE_LENGTH 32
#define MAX_NOTES_LENGTH 160

typedef enum {
  TASK_CATEGORY_CREATIVE,
  TASK_CATEGORY_WORK,
  TASK_CATEGORY_PERSONAL,
  TASK_CATEGORY_DEVELOPMENT,
  TASK_CATEGORY_WELLBEING,
  TASK_CATEGORY_HOME,
  TASK_CATEGORY_UNKNOWN
} TaskCategory;

typedef struct {
  char id[MAX_ID_LENGTH];
  char title[MAX_TITLE_LENGTH];
  char list[MAX_LIST_LENGTH];
  char due[MAX_DUE_LENGTH];
  char notes[MAX_NOTES_LENGTH];
  TaskCategory category;
  bool completed;
  bool overdue;
} Task;

void task_store_init(void);
void task_store_deinit(void);
void task_store_clear(void);
int task_store_count(void);
void task_store_set_count(int count);
const Task *task_store_get(int index);
void task_store_set_task(int index, const char *id, const char *title, const char *list, const char *due, bool completed, bool overdue);
void task_store_set_task_notes(int index, const char *notes);
void task_store_set_completed(int index, bool completed);
int task_store_completed_count(void);
bool task_store_all_completed(void);
int task_store_next_incomplete(int from, int direction);
TaskCategory task_store_category_for_list(const char *list);

#endif
