const gpio = require('onoff').Gpio;


class ScreenCommand {
    static count = 0;

    constructor() {
        this.index = ++ScreenCommand.count;
        this.time = 0;
    }
}


class Command {
    constructor(name, type, retain = true, onchange = null, opts = null) {
        this.name = name;
        this.type = type;
        this.onchange = onchange;
        this.retain = retain;
        this.opts = opts;
        this.value = null;
        this.rawValue = null;
        this.at = new Date(2000, 1, 1);
        this.online = 0;
        this.screen = new ScreenCommand();
        if (this.opts != null) {
            if (this.opts.GPO) {
                for (var j = 0; j < this.opts.GPO.length; j++) {
                    this.opts.GPO[j] = new gpio(this.opts.GPO[j], 'out');
                }
            }
            if ('safe' in this.opts) {
                this.value = this.opts.safe;
                this.rawValue = "" + this.value;
                if (this.opts.GPO && this.opts.GPO instanceof Array)
                    this.opts.GPO[0].writeSync(this.value);
            }
        }
    }

    shutdown() {
        if (this.opts && 'safe' in this.opts) {
            this.value = this.opts.safe;
            this.rawValue = "" + this.value;
            if (this.opts.GPO && this.opts.GPO instanceof Array)
                this.opts.GPO[0].writeSync(this.value);
        }
    }

    getValue(fixed = null) {
        if (this.value == null)
            return null;

        switch (this.type) {
            case 1:
                return parseInt(this.value);
            case 2:
                if (fixed)
                    return parseFloat(this.value).toFixed(fixed);
                return parseFloat(this.value);
            case 3:
            case 4:
                return this.value;
        }

        return null;
    }

    setValue(value) {
        if (value != this.rawValue) {
            this.rawValue = "" + value;
            this.online = 1;
            this.at = Date.now();
            switch (this.type) {
                case 1:
                    this.value = parseInt(value);
                    return true;
                case 2:
                    this.value = parseFloat(value);
                    return true;
                case 3:
                case 4:
                    this.value = value;
                    return true;
            }
        }
        return false;
    }

    haveValue() {
        return this.value != null;
    }

    doSimulation() {
        if (this.online == 1)
            return false;

        if (this.type == 2) {
            if (/volt/.test(this.name)) {
                this.value = parseFloat(this.rawValue = (Math.random() * 4 + 11).toFixed(2));
            } else if (/amp/.test(this.name)) {
                this.value = parseFloat(this.rawValue = (Math.random() * 50 - 25).toFixed(2));
            } else if (/back/.test(this.name)) {
                this.value = parseFloat(this.rawValue = (Math.random() * 50).toFixed(2));
            } else if (/level/.test(this.name)) {
                this.value = parseFloat(this.rawValue = (Math.random() * 100).toFixed(2));
            } else if (/temp/.test(this.name)) {
                this.value = parseFloat(this.rawValue = (Math.random() * 40 - 10).toFixed(2));
            } else if (/hum/.test(this.name)) {
                this.value = parseFloat(this.rawValue = (Math.random() * 100).toFixed(2));
            }
            this.at = Date.now();
            this.online = 2;
            return true;
        }
        return false;
    }
}

module.exports = Command