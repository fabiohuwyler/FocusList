var messageQueue = [];
var sending = false;

function sendNext() {
  if (messageQueue.length === 0) {
    sending = false;
    return;
  }
  sending = true;
  var message = messageQueue.shift();
  Pebble.sendAppMessage(message, sendNext, function(e) {
    console.log('FocusList: message send failed', e);
    sending = false;
  });
}

function queueMessage(message) {
  messageQueue.push(message);
  if (!sending) {
    sendNext();
  }
}

function sendTasks() {
  messageQueue = [];
  sending = false;

  queueMessage({
    'MESSAGE_TYPE': 1,
    'TASK_COUNT': MOCK_TASKS.length
  });

  for (var i = 0; i < MOCK_TASKS.length; i++) {
    var task = MOCK_TASKS[i];
    queueMessage({
      'MESSAGE_TYPE': 2,
      'TASK_INDEX': i,
      'TASK_ID': task.id,
      'TASK_TITLE': task.title,
      'TASK_LIST': task.list,
      'TASK_DUE': task.due,
      'TASK_COMPLETED': task.completed ? 1 : 0,
      'TASK_OVERDUE': task.overdue ? 1 : 0
    });
    queueMessage({
      'MESSAGE_TYPE': 3,
      'TASK_INDEX': i,
      'TASK_NOTES': task.notes
    });
  }

  queueMessage({
    'MESSAGE_TYPE': 4
  });
}
