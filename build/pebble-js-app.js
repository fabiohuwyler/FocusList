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
Pebble.addEventListener('ready', function() {
  console.log('FocusList: JS ready');
});

Pebble.addEventListener('appmessage', function(e) {
  var payload = e.payload;
  if (payload['REQUEST_TASK'] === 1) {
    sendTasks();
  }
});
var MOCK_TASKS = [
  {
    id: '1',
    title: 'Finish the blog post',
    list: 'Creative',
    due: 'Today · 18:00',
    notes: 'Review the final paragraph, add links to the three Pebble projects, and check both language versions before publishing.',
    completed: false,
    overdue: false
  },
  {
    id: '2',
    title: 'Reply to Martina',
    list: 'Work',
    due: 'Today',
    notes: 'Confirm the time for the next coaching session and mention the topics that would be useful to discuss.',
    completed: false,
    overdue: false
  },
  {
    id: '3',
    title: 'Buy coffee beans',
    list: 'Personal',
    due: 'Today',
    notes: 'Get the medium roast from the small shop near the station. Check whether they also have the decaf beans.',
    completed: false,
    overdue: false
  },
  {
    id: '4',
    title: 'Upload the new build',
    list: 'Development',
    due: 'Yesterday',
    notes: 'Build the latest package, test it on rectangular and round emulators, update the version number, and upload the PBW.',
    completed: false,
    overdue: true
  },
  {
    id: '5',
    title: 'Take a short walk',
    list: 'Wellbeing',
    due: 'Today · 16:30',
    notes: 'Ten minutes is enough. No destination required. Leave the phone in your pocket and notice what is happening around you.',
    completed: false,
    overdue: false
  },
  {
    id: '6',
    title: 'Plan newsletter topics',
    list: 'Creative',
    due: 'Tomorrow',
    notes: 'Collect possible stories about AI, Switzerland, LGBTQ+ news, and the creator economy. Choose only the strongest ideas.',
    completed: false,
    overdue: false
  },
  {
    id: '7',
    title: 'Water the plants',
    list: 'Home',
    due: 'Tomorrow',
    notes: 'Check the soil before watering. The large plant probably needs less water than the smaller plants near the window.',
    completed: false,
    overdue: false
  }
];
