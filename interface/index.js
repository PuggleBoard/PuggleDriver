#!/usr/bin/env node

var app = require('http').createServer(handler)
  , io = require('socket.io').listen(app)
  , fs = require('fs')

app.listen(8081);

var child=require('child_process');
var prog;

function handler (req, res) {
	fs.readFile(req.url.substr(1),
  //fs.readFile(__dirname + '/index.html',
  function (err, data) {
    if (err) {
      res.writeHead(500);
      return res.end('Error loading index.html');
    }

    res.writeHead(200);
    res.end(data);
  });
}



io.sockets.on('connection', function (socket) {
	socket.emit('status', {stat: 'idle'});
	socket.on('capture', function (data) {
    console.log(data);
    // perform the capture
    
    prog=child.exec('../apps/bin/prus'), function (error, stdout, stderr) {
    	if (error) {
     		console.log(error.stack);
    		console.log('Error code: '+error.code);
     		console.log('Signal received: '+error.signal);
   		}
   	}
    // wait for it to finish (we should see an 'exit' event)
		prog.on('exit', function (code) {
			socket.emit('status', {stat: 'captured'});
			var fs2=require('fs');
			fs2.readFileSync("data.csv").toString().split('\n').forEach(function (line) {
				socket.emit('results', {measurement: line});
			});
		});
		    
    
    
    
    
    
  });

});


