// From Getting Started With node.js and socket.io 
// http://codehenge.net/blog/2011/12/getting-started-with-node-js-and-socket-io-v0-7-part-2/
"use strict";

var http = require('http'),
    url = require('url'),
    fs = require('fs'),
    exec = require('child_process').exec,
    server,
    connectCount = 0,	// Number of connections to server
    PushTimer = 100,
    ainNum_ys = 6,
    ainNum_xs = 4;

server = http.createServer(function (req, res) {
// server code
    var path = url.parse(req.url).pathname;
    console.log("path: " + path);
    switch (path) {
    case '/':
        res.writeHead(200, {'Content-Type': 'text/html'});
        res.write('<h1>Hello!</h1>Try<ul><li><a href="/Joystick.html">Button Box Joystick</a></li></ul>');

        res.end();
        break;

    default:		// This is so all the files will be sent.
        fs.readFile(__dirname + path, function (err, data) {
            if (err) {return send404(res); }
//            console.log("path2: " + path);
            res.write(data, 'utf8');
            res.end();
        });
        break;

    }
});

var send404 = function (res) {
    res.writeHead(404);
    res.write('404');
    res.end();
};

server.listen(8081);

// socket.io, I choose you
var io = require('socket.io').listen(server);
io.set('log level', 2);

// on a 'connection' event
io.sockets.on('connection', function (socket) {
    var frameCount = 0;	// Counts the frames from arecord
    var lastFrame = 0;	// Last frame sent to browser
    console.log("Connection " + socket.id + " accepted.");
//    console.log("socket: " + socket);

    // now that we have our connected 'socket' object, we can 
    // define its event handlers

    // Make sure some needed files are there
    // The path to the analog devices changed from A5 to A6.  Check both.
    var ainPath = "/sys/devices/platform/omap/tsc/";
//    if(!fs.existsSync(ainPath)) {
//        ainPath = "/sys/devices/platform/tsc/";
//        if(!fs.existsSync(ainPath)) {
//            throw "Can't find " + ainPath;
//        }
//    }
  

    // Send value every time a 'message' is received.
    socket.on('ain_y', function (ainNum_y) {
//        var ainPath = "/sys/devices/platform/omap/tsc/ain" + ainNum;
	ainNum_ys = ainNum_y;        
	fs.readFile(ainPath + "ain" + ainNum_y, 'base64', function(err, data) {
            if(err) throw err;
            socket.emit('ain_y', data);
	    
//            console.log('emitted ain: ' + data);
        });
    });

    socket.on('ain_x', function (ainNum_x) {
//        var ainPath = "/sys/devices/platform/omap/tsc/ain" + ainNum;
	ainNum_xs = ainNum_x;
        fs.readFile(ainPath + "ain" + ainNum_x, 'base64', function(err, data) {
            if(err) throw err;
            socket.emit('ain_x', data);
//            console.log('emitted ain: ' + data);
        });
    });


    socket.on('disconnect', function () {
        console.log("Connection " + socket.id + " terminated.");
        connectCount--;
        if(connectCount === 0) {
        }
        console.log("connectCount = " + connectCount);
    });

    connectCount++;
    console.log("connectCount = " + connectCount);

    function Push() {
	fs.readFile(ainPath + "ain" + ainNum_ys, 'base64', function(err, data) {
		if(err) throw err;
		socket.emit('ain_y', data);
	});
	fs.readFile(ainPath + "ain" + ainNum_xs, 'base64', function(err, data) {
		if(err) throw err;
		socket.emit('ain_x', data);
	});

	setTimeout(Push, PushTimer);
    }

    Push();

});

