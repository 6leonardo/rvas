const mysql = require('mysql');
const process = require('process');
const gpio = require('onoff').Gpio;
const express = require('express');
const bodyParser = require('body-parser');
const session = require('express-session');
const app = express();
const port = process.argv[2] || 8888;
const screenType = parseInt(process.argv[3] || 1);
const http = require('http').Server(app);
var term = require('terminal-kit').terminal;
/*
const io = require('socket.io')(http);
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

class Automation {

    constructor(commands, liveDB) {
        this.commands = {};
        this.liveDB = liveDB;
        this.started = 0;
        for (var i = 0; i < commands.length; i++) {
            var command = commands[i];
            if (command.opts && command.opts.GPO) {
                for (var j = 0; j < command.opts.GPO.length; j++) {
                    command.opts.GPO[j] = new gpio(command.opts.GPO[j], 'out');
                }
            }
            this.commands[command.name] = command;
        }
        var me = this;
        wait(() => liveDB.started).then(() => me.startup());
    }

    startup() {
        for (const [name, command] of Object.entries(this.commands))
            if (command.onchange) {
                this.update(name, this.liveDB.readMeasure(name));
                screen.update(name);
            }
        this.started = 1;
    }

    shutdown() {
        for (const [name, command] of Object.entries(this.commands))
            if (command.opts && 'safe' in command.opts && command.onchange) {
                var safe = command.opts.safe;
                this.liveDB.writeMeasure(name, safe)
                this.update(name, safe);
            }
        this.started = 0;
    }

    update(name, value) {
        var command = null;
        if (command = this.commands[name]) {
            if (command.onchange)
                command.onchange.apply(this, [command, value]);
        }
    }

    normalDigital(command, value) {
        command.opts.GPO[0].writeSync(parseInt(value));
    }


};

var commands = [{ name: 'water/1/level', onchange: null },
    { name: 'water/2/level', onchange: null },
    { name: 'water/valve', onchange: Automation.prototype.normalDigital, opts: { GPO: [4], safe: 0 } },
    { name: 'water/pump', onchange: Automation.prototype.normalDigital, opts: { GPO: [17], safe: 0 } },
    { name: 'water/1/temp', onchange: null },
    { name: 'water/2/temp', onchange: null },
    { name: 'water/1/unload', onchange: Automation.prototype.normalDigital, opts: { GPO: [27], safe: 0 } },
    { name: 'water/2/unload', onchange: Automation.prototype.normalDigital, opts: { GPO: [22], safe: 0 } },
    { name: 'sensor/int/temp', onchange: null },
    { name: 'sensor/int/hum', onchange: null },
    { name: 'sensor/ext/temp', onchange: null },
    { name: 'sensor/ext/hum', onchange: null },
    { name: 'battery/1/volt', onchange: null },
    { name: 'battery/1/amp', onchange: null },
    { name: 'battery/1/back', onchange: null },
    { name: 'battery/2/volt', onchange: null },
    { name: 'battery/2/amp', onchange: null },
    { name: 'battery/3/volt', onchange: null },
    { name: 'battery/3/amp', onchange: null },
    { name: 'solar/volt', onchange: null },
    { name: 'solar/amp', onchange: null },
    { name: 'gas/1/level', onchange: null },
    { name: 'gas/2/level', onchange: null },
    { name: 'lights', onchange: Automation.prototype.normalDigital, opts: { GPO: [5], safe: 0 } },
    { name: 'light/extern', onchange: null },
    { name: 'light/living', onchange: null },
    { name: 'light/kitchen', onchange: null },
    { name: 'generale', onchange: Automation.prototype.normalDigital, opts: { GPO: [6], safe: 0 } },
    { name: 'inverter', onchange: Automation.prototype.normalDigital, opts: { GPO: [13], safe: 0 } },
    { name: 'fridge/on', onchange: null },
    { name: 'heater/on', onchange: null },
    { name: 'airc/on', onchange: null },
    { name: 'fans/1', onchange: Automation.prototype.normalDigital, opts: { GPO: [19], safe: 0 } },
    { name: 'fans/2', onchange: Automation.prototype.normalDigital, opts: { GPO: [26], safe: 0 } },
    { name: 'fans/3', onchange: Automation.prototype.normalDigital, opts: { GPO: [18], safe: 0 } },
    { name: 'fans/4', onchange: Automation.prototype.normalDigital, opts: { GPO: [23], safe: 0 } },
    { name: 'sensor/ext/gas/1', onchange: null },
    { name: 'sensor/int/gas/1', onchange: null },
];

class LiveDB {

    constructor() {
        this.measures = [];
        this.fields = [];
        this.index = {};
        this.started = 0;
        var me = this;
        cn.query('select * from measures', function(err, result, fields) {
            if (err)
                screen.log(err);
            else {
                me.measures = result;
                me.fields = fields;
                me.index = {};
                //screen.log(result);
                for (var i = 0; i < result.length; i++)
                    me.index[result[i].name] = i;
                me.started = 1;
            }
        })
    }

    writeMeasure(key, value) {
        if (key in this.index) {
            var variable = this.measures[this.index[key]]
            switch (variable.type) {
                case 1:
                    variable.v_int = parseInt(value);
                    cn.query('update measures set v_int=?, update_time=CURRENT_TIMESTAMP() where id=?', [variable.v_int, variable.id], this.db_error);
                    if (variable.record == 1)
                        cn.query('insert into history (measureid, v_int, update_time ) values( ?,?,CURRENT_TIMESTAMP())', [variable.id, variable.v_int], this.db_error);
                    variable.interval = Date.now() - variable.update_time;
                    variable.update_time = Date.now();
                    return true;
                case 2:
                    variable.f_float = parseFloat(value);
                    cn.query('update measures set v_float=?, update_time=CURRENT_TIMESTAMP() where id=?', [variable.v_float, variable.id], this.db_error);
                    if (variable.record == 1)
                        cn.query('insert into history (measureid, v_float, update_time ) values( ?,?,CURRENT_TIMESTAMP())', [variable.id, variable.v_float], this.db_error);
                    variable.interval = Date.now() - variable.update_time;
                    variable.update_time = Date.now();
                    return true;
                case 3:
                    variable.v_string = "" + value;
                    cn.query('update measures set v_string=?, update_time=CURRENT_TIMESTAMP() where id=?', [variable.v_string, variable.id], this.db_error);
                    if (variable.record == 1)
                        cn.query('insert into history (measureid, v_string, update_time ) values( ?,?,CURRENT_TIMESTAMP()', [variable.id, variable.v_string], this.db_error);
                    variable.interval = Date.now() - variable.update_time;
                    variable.update_time = Date.now();
                    return true;
            }
        }
        return false;
    }

    addMeasure(key, value) {
        if (key in this.index) {
            var variable = this.measures[this.index[key]]
            variable.online = 1;
            var done = this.writeMeasure(key, value);
            screen.update(key);
            if (done)
                automation.update(key, value);
            return done;
        }
        return false;
    }

    readMeasure(key) {
        if (key in this.index) {
            var variable = this.measures[this.index[key]]
            return this.getValue(variable);
        }
        return null;
    }

    getValue(el) {
        switch (el.type) {
            case 1:
                return el.v_int;
            case 2:
                return el.v_float;
            case 3:
                return el.v_string;
            default:
                return null;
        }
    }

    readMeasuresByTime(time) {
        var result = { measures: [], time: time };
        var lastUpdate = time;
        var updated = this.measures.filter(function(measure) { return measure.update_time > time })
        for (var i = 0; i < updated.length; i++) {
            var el = updated[i];
            var value = this.getValue(el);
            result.measures.push({ name: el.name, value: value });
            if (el.update_time > lastUpdate) {
                lastUpdate = el.update_time
            }
        }
        result.time = lastUpdate;
        return result;
    }

    db_error(err) {
        if (err)
            screen.log(err);
    }

    readAll() {
        var html = "<table>";
        for (var i = 0; i < this.measures.length; i++) {
            var variable = this.measures[i]
            var value = null;
            switch (variable.type) {
                case 1:
                    value = variable.v_int;
                    break;
                case 2:
                    value = variable.v_float;
                    break;
                case 3:
                    value = variable.v_string;
                    break;
            }
            var date = new Date(variable.update_time).toLocaleTimeString();
            var online = (!!variable.online) && variable.online == 1;
            html += `<tr><td>${variable.name}</td><td>${value}</td><td>${online}</td><td>${date}</td></tr>`;
        }
        return html + "</table>";
    }

    simulate() {
        var time = Date.now();
        for (var i = 0; i < this.measures.length; i++) {
            var el = this.measures[i];

            if ((!!el.online) && el.online == 1)
                continue;

            if (el.type == 2) {
                if (/volt/.test(el.name)) {
                    el.v_float = Math.random() * 4 + 11;
                }
                if (/amp/.test(el.name)) {
                    el.v_float = Math.random() * 50 - 25;
                }
                if (/back/.test(el.name)) {
                    el.v_float = Math.random() * 50;
                }
                if (/level/.test(el.name)) {
                    if (/gas/.test(el.name))
                        el.v_float = Math.random() * 10;
                    else
                        el.v_float = Math.random() * 100;
                }
                if (/temp/.test(el.name)) {
                    el.v_float = Math.random() * 40 - 10;
                }
                if (/hum/.test(el.name)) {
                    el.v_float = Math.random() * 100;
                }
                el.update_time = time;
                el.online = 2;
                screen.update(el.name);

            }

        }
    }
}

class Console {
    constructor(liveDB) {
        this.started = 0;
        this.liveDB = liveDB;
        this.last = 0;
        this.logs = ["", "", "", "", ""];
        this.times = [];
        this.n_log = 5;
        this.i_log = 0;
        this.timer = 0;
        if (screenType == 2) {
            term.clear();
            var me = this;
            wait(() => liveDB.started).then(() => me.startup());
        }

    }

    startup() {
        if (screenType == 2) {
            this.last = this.liveDB.measures.length + 1;
            for (var i = 0; i < this.liveDB.measures.length; i++) {
                this.update(this.liveDB.measures[i].name);
                this.times.push(0);
            }

            term.moveTo(1, this.last).bgWhite.black.eraseLine();
            term(" -- log --");
            term.styleReset();
            this.started = 1;
            this.timer = setInterval(() => {
                var time = new Date();
                for (var i = 0; i < this.times.length; i++) {
                    if (this.times[i]) {
                        if (time - this.times[i] > 1000) {
                            term.moveTo(30, i + 1);
                            term(" ");
                            this.times[i] = 0;
                        }
                    }
                }
            }, 1000);
        }
    }

    shutdown() {
        if (screenType == 2) {
            clearInterval(this.timer);
            term.clear();

        }
    }

    update(name) {
        var i = this.liveDB.index[name];
        var el = this.liveDB.measures[i];
        switch (screenType) {
            case 2:
                this.times[i] = new Date();
                term.moveTo(1, i + 1);
                term(el.name);

                term.moveTo(30, i + 1);
                term("X");

                if (el.online) {
                    term.moveTo(32, i + 1);
                    term(el.online == 1 ? "o" : "s");
                }
                term.moveTo(34, i + 1);
                if (el.type == 2)
                    term(this.liveDB.getValue(el).toFixed(2) + " ");
                else
                    term("" + this.liveDB.getValue(el));

                term.moveTo(50, i + 1);
                term(new Date(el.update_time).toLocaleTimeString() + "  ")
                if ('interval' in el) {
                    term.moveTo(70, i + 1);
                    term("" + el.interval + "  ")
                }
                break;
            case 3:
                var text = (el.online == 1 ? "o " : el.online == 2 ? "s " : "  ") + el.name + " ";
                if (el.type == 2)
                    text += this.liveDB.getValue(el).toFixed(2)
                else
                    text += this.liveDB.getValue(el);
                text += new Date(el.update_time).toLocaleTimeString();
                if ('interval' in el)
                    text += " " + el.interval;
                console.log(text);
                break;
        }
    }

    log(text) {
        if (screenType == 2) {
            if (!this.started)
                return;
            term.white.bgBlack();
            this.logs[this.i_log] = text;
            for (var i = 0; i < this.n_log; i++) {
                var index = ((this.n_log + this.i_log - i) % this.n_log);
                term.moveTo(1, this.last + 1 + i);
                term.eraseLine();
                term.moveTo(1, this.last + 1 + i);
                term(this.logs[index]);
            }
            this.i_log = (this.i_log + 1) % this.n_log;
            term.styleReset();
        } else {
            console.log(text);
        }
    }
}

var cn = mysql.createConnection({
    host: 'localhost',
    user: 'rv',
    password: '32camper.8',
    database: 'RV'
});

cn.connect();

var liveDB = new LiveDB();
var automation = new Automation(commands, liveDB);
var screen = new Console(liveDB);

function onexit() {
    screen.log("clean exit");
    automation.shutdown();
    screen.shutdown();
    //term.clear();
    wait(() => !automation.started).then(() => process.exit(1));
}

process.on('SIGINT', onexit);
process.on('SIGTERM', onexit);
/*
app.get('/add/:name/:value', (req, res) => {
    var ret = liveDB.addMeasure(req.params.name, req.params.value);
    res.json({ status: ret });
    //res.send('Hello World!')
})
*/
var sessions = 0;
var pages = 0;
var myLogger = function(req, res, next) {
        //screen.log(req.session);

        if (!req.session.content) {
            req.session.content = 1;
            //req.session.save();
            sessions++;
            screen.log((new Date()).toLocaleTimeString() + " Sessions: " + sessions + " Pages: " + pages);
        } else {
            req.session.content += 1;
            //req.session.save();
            pages++;
        }
        next();
    }
    /*
    type:'post',
    dataType:'json',
    */
    /*
    app.use(function(req, res, next) {
        // Website you wish to allow to connect
        res.setHeader('Access-Control-Allow-Origin', '*'); //req.headers.origin);
        // Request methods you wish to allow
        res.setHeader('Access-Control-Allow-Methods', 'GET, POST, PUT, PATCH, DELETE');
        // Request headers you wish to allow
        res.setHeader('Access-Control-Allow-Headers', 'X-Requested-With,content-type, Authorization');
        // Set to true if you need the website to include cookies in the requests sent
        // to the API (e.g. in case you use sessions)
        res.setHeader('Access-Control-Allow-Credentials', 'true');
        // Pass to next layer of middleware
        next();
    });
    app.set('trust proxy', 1)
    app.use(cors({ credentials: true }));
    */
app.use(bodyParser.urlencoded({ extended: true }));
app.use(bodyParser.json()); // to support JSON-encoded bodies
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

app.use(myLogger);

app.get('/add', (req, res) => {
    var ret = liveDB.addMeasure(req.query.name, req.query.value);
    res.json({ status: ret });
    //res.send('Hello World!')
})

app.get('/get', (req, res) => {
    var value = liveDB.readMeasure(req.query.name);
    res.json({ status: value != null, value: value });
    //res.send('Hello World!')
})

app.get('/get_by_time', (req, res) => {
    var time = new Date(req.query.time);
    var result = liveDB.readMeasuresByTime(time);
    //screen.log(result);
    res.json({ status: 1, result: result });
    //res.send('Hello World!')
})

app.get('/list', (req, res) => {
    res.type('html');
    res.send(liveDB.readAll());
})

//app.use('/static', express.static(__dirname + '/public'));
//app.use('/', express.static('/var/www/html/'));
app.use('/', express.static(__dirname + '/public'));

/*
//for use socket in the client

var count = 0
var ipsConnected = [];

io.on('connection', function(socket) {
    var ipAddress = socket.handshake.address;
    if (!ipsConnected.hasOwnProperty(ipAddress)) {
        ipsConnected[ipAddress] = 1;
        count++;
        //socket.emit('counter', {count:count});
    }

    screen.log("client is connected: " + count);
    socket.on('disconnect', function() {
        if (ipsConnected.hasOwnProperty(ipAddress)) {
            delete ipsConnected[ipAddress];
            count--;
            screen.log("client is connected: " + count);
            //socket.emit('counter', {count:count});
        }
    });
})
*/

function main() {
    setInterval(function() { liveDB.simulate(); }, 5000);
    http.listen(port, () => screen.log(`listening at http: //localhost:${port}`))
}

wait(() => liveDB.started && automation.started).then(main);