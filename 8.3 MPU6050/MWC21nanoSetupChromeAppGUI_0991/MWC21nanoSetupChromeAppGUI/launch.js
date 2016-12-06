chrome.app.runtime.onLaunched.addListener(function() {
  chrome.app.window.create('index.html', {
	frame: 'chrome',
	id: 'main-window',
    width: 800,
    height: 600
  });
});