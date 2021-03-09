const gpio = require('onoff').Gpio;
const events = require('events');

class Automation {

    constructor(commands, mqtt) {
        this.mqtt = mqtt;
        this.commands = commands;
        this.index = {};
        this.started = 0;
        this.emitter = new events.EventEmitter();
        for (var i = 0; i < commands.length; i++) {
            var cmd = commands[i];
            this.index[cmd.name] = i;
        }
        /*
        var me = this;
        wait(() => liveDB.started).then(() => me.startup());
        */
    }

    startup() {
        /*
        for (const [name, command] of Object.entries(commands))
            if (command.onchange) {
                this.update(name, liveDB[name]);
                //screen.update(name);
            }
        this.started = 1;
        */
        this.mqtt_sync();
        this.started = 1;
    }

    mqtt_sync() {
        for (var i = 0; i < this.commands.length; i++) {
            var cmd = this.commands[i];
            if (cmd.haveValue()) {
                var value = cmd.getValue();
                this.mqtt.publish(cmd.name, "" + value);
                this.emitter.emit('publish', cmd.name, value);
            }
        }
    }

    simulate() {
        for (var i = 0; i < this.commands.length; i++) {
            var cmd = this.commands[i];
            if (cmd.doSimulation()) {
                var value = cmd.getValue();
                this.mqtt.publish(cmd.name, "sss " + value);
                //this.emitter.emit('publish', cmd.name, value);
            }
        }
    }

    shutdown() {
        for (var i = 0; i < this.commands.length; i++) {
            var cmd = this.commands[i];
            cmd.shutdown();
        }
        this.mqtt_sync();

        //for (const [name, command] of Object.entries(commands))
        this.started = 0;
    }

    on(event, fn) {
        return this.emitter.on(event, fn);
    }

    update(name, value) {
        var cmd = name in this.index ? this.commands[this.index[name]] : null;
        if (cmd && value != cmd.rawValue) {
            cmd.setValue(value);
            if (cmd.onchange)
                cmd.onchange.apply(this, [cmd]);
        }
    }

    normalDigital(command) {
        command.opts.GPO[0].writeSync(parseInt(command.getValue()));
    }

};

module.exports = Automation
