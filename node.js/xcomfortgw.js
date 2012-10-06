var http = require('http');
var net = require('net');
var url = require('url');

var requests = [];

var connection = null;

setInterval(function() {
	if (connection == null) {
		connection = net.createConnection(1050, '127.0.0.1', function() {
			connection.on('data', function(data) {
				console.log('requests: ' + requests.length + ', connection exists? ' + (connection != null));
				if (data == 'ok') {
					for (var i in requests) {
						requests[i].end('ok');
						requests.splice(i, 1);
					}
				}
				else {
					for (var i in requests) {
						requests[i].writeHead(408);
						requests[i].end();
						requests.splice(i, 1);
					}
				}
			});
		});
		connection.setEncoding('utf8');
		connection.on('error', function() {
			console.log('connection failed');
		});
		connection.on('close', function() {
			connection = null;
		});
		connection.setNoDelay(true);
	}
}, 1000);

var httpServer = http.createServer(function (req, res) {
	var urlParsed = url.parse(req.url, true);

	switch (urlParsed.pathname) {
		case '/set_dim':
		var serial = urlParsed.query['serial'];
		var level = urlParsed.query['level'];

		if (serial.length > 10 || level.length > 3) {
			res.writeHead(500);
			res.end();
		}

		if (connection != null && requests.length == 0) {
			connection.write('0;' + serial + ';' + level + '.', 'utf8', function() {
				
			});
		}

		requests.push(res);
		setTimeout(function() {
			if (requests.indexOf(res) > -1) {
				res.writeHead(408);
				res.end();
				requests.splice(requests.indexOf(res), 1);
			}
		}, 1000);
		break;
		
		default:
		res.writeHead(404);
		res.end();
		break;
	}
});

httpServer.listen(8082, '0.0.0.0');

setInterval(function() {
	console.log('requests: ' + requests.length + ', connection exists? ' + (connection != null));
}, 1000);
