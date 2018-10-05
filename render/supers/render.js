///Phantomjs renderer 
//
//
var system = require('system');
var args = system.args;

var url, render_dir, frame_count, frame_skip;

if (args.length == 5) {
  url = args[1];
  render_dir = args[2];
  frame_count = args[3];
  frame_skip = args[4];
} else {
  console.log(args[0] + ' {URL} {Render Directory} {Frame count} {skip frame}');
  phantom.exit();
}

var page = require('webpage').create();
page.viewportSize = { width: 1080, height: 650 };
page.open(url, function (status) {
    if (status !== 'success') {
        console.log('Unable to access the network!');
    } else {

    console.log('page opened');
        page.evaluate(function () {
            var body = document.body;
            body.style.backgroundColor = '#fff';
        });

      setTimeout(function() {
          // Initial frame
          var frame = 0;
          // Add an interval every 25th second
          setInterval(function() {
            // Render an image with the frame name
            console.log('rendered '+frame);
            frame++;

            if(frame > frame_skip)
              page.render(render_dir + '/frame'+(frame - frame_skip)+'.png', { format: "png" });

            // Exit after 50 images
            if(frame > frame_count) {
              phantom.exit();
            }
          }, 25);
        }, 100);

        console.log('page close');
    }
});

page.onError = function (msg, trace) {
    console.log(msg);
    trace.forEach(function(item) {
        console.log('  ', item.file, ':', item.line);
    });
};



