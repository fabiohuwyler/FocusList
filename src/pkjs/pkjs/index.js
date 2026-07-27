Pebble.addEventListener('ready', function() {
  console.log('FocusList: JS ready');
});

Pebble.addEventListener('appmessage', function(e) {
  var payload = e.payload;
  if (payload['REQUEST_TASK'] === 1) {
    sendTasks();
  }
});
