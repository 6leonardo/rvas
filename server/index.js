const mysql = require('mysql');
const mqtt = require('mqtt');
const process = require('process');
const gpio = require('onoff').Gpio;
const express = require('express');
//const bodyParser = require('body-parser');
const session = require('express-session');
const app = express();
const port = process.argv[2] || 8888;
const screenType = parseInt(process.argv[3] || 1);
const http = require('http').Server(app);

const io = require('socket.io')(http);
const Command = require("./command.js");
const Console = require("./console.js");
const Automation = require("./automation.js");
var commands = require("./commands.js");
/*
const path = require('path');
const cors = require('cors')
const { timeStamp, clear } = require('console');
const fileStore = require('session-file-store')(session);
*/
/*
function sleep(ms) {
    return new Promise(resolve => setTimeout(resolve, ms));
    }
  console.log("Hello");
sleep(2000).then(() => { console.log("World!"); });
*/

function wait(test) {
    return new Promise(resolve => {
        var timer = setInterval(function() {
            if (test()) {
                clearInterval(timer);
                resolve();
            }
        }, 1000)
    })
}

/*
var cn = mysql.createConnection({
    host: 'localhost',
    user: 'rv',
    password: '32camper.8',
    database: 'RV'
});
cn.connect();
*/

var mqtt_client = mqtt.connect({ host: 'localhost', port: 1883, username: "pi", password: "demodemo" });
//var automation = new Automation(commands, mqtt_client);
//var screen = new Console(commands, screenType, automation.index);
var automation = null;
var screen = null;
var sockets = [];
var mqtt_connected = false;

mqtt_client.on('connect', () => {
    mqtt_connected = true;
})
mqtt_client.on('error', error => {
    if (error) {
        console.log(error);
        process.exit(1);
    }
})
mqtt_client.on('message', (topic, message) => {
    //screen.log(`${topic} => ${message}`);
    var index = automation.index[topic];
    if (index !== undefined) {
        var cmd = commands[index];
        var sim = false;
        if (/^sss /.test(message)) {
            //if ((typeof message) == "string")
            //    message = message.substring(4);
            sim = true;
        }
        screen.log(topic + " " + message+" "+sim);
        if (cmd.rawValue != message && !sim) {
            automation.update(topic, message);
        }
        var value = cmd.getValue();
        for (var i = 0; i < sockets.length; i++) {
            try {
                sockets[i].emit('publish', topic, value);
            } catch (e) {
                screen.log(e);
            }
        }
        screen.update(topic);
    }
})


function onexit() {
    if (automation != null) {
        automation.shutdown();
        screen.shutdown();
    }
    console.log("clean exit");
    //term.clear();
    wait(() => automation == null || !automation.started).then(() => process.exit(1));
}

process.on('SIGINT', onexit);
process.on('SIGTERM', onexit);

app.use(express.json()); // to support JSON-encoded bodies
app.use(express.urlencoded({ extended: true })); // to support URL-encoded   bodies
app.use(session({
    //store: new fileStore({ path: __dirname + '/sessions' }),
    //    name: 'session-cookie-id',
    secret: '43aslklksòdlkòldksòldas',
    resave: false,
    saveUninitialized: true,
    //store: new FileStore(),
    //cookie: { secure: false, httpOnly: false, maxAge: 24 * 60 * 60 * 1000 }
}));

app.get('/push', (req, res) => {
    mqtt_client.publish(req.query.topic, req.query.message, 'retain' in req.query ? { retain: req.query.retain } : {});
    res.json({ status: ret });
})

app.get('/pull', (req, res) => {
    var value = req.query.name in liveDB ? liveDB[req.query.name].message : null;
    res.json({ status: value != null, value: value });
})

//app.use('/static', express.static(__dirname + '/public'));
//app.use('/', express.static('/var/www/html/'));
app.use('/', express.static(__dirname + '/public'));

io.on('connection', function(socket) {
    var index = sockets.findIndex(el => el === socket);
    if (index == -1) {
        sockets.push(socket);
    }

    screen.log("new connection " + index + " " + sockets.length);

    socket.on('publish', (topic, message) => {
        var ops = {};
        if (topic in automation.index)
            ops = { retain: commands[automation.index[topic]].retain };
        mqtt_client.publish(topic, "" + message, ops);
    });

    socket.on('start', () => {
        for (var i = 0; i < commands.length; i++) {
            socket.emit('publish', commands[i].name, commands[i].getValue());
        }
    });

    socket.on('disconnect', () => {
        var index = sockets.findIndex(el => el === socket);
        if (index != -1)
            sockets.splice(index, 1);
        screen.log("disconnect " + index + " " + sockets.length);
    });
})



function main() {
    automation = new Automation(commands, mqtt_client);
    screen = new Console(commands, screenType, automation.index);
    automation.on('publish', (topic, message) => screen.update(topic, message));
    mqtt_client.subscribe('#', err => {
        if (err)
            console.log(err);
    });
    automation.startup();

    setInterval(function() { automation.simulate(); }, 5000);
    http.listen(port, () => screen.log(`listening at http: //localhost:${port}`))
}

wait(() => mqtt_connected).then(main);
//wait(() => liveDB.started && automation.started).then(main);
