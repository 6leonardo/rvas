var term = require('terminal-kit').terminal;

class Console {
    constructor(commands, screenType, index) {
        this.index = index;
        this.screenType = screenType;
        this.commands = commands;
        this.logs = ["", "", "", "", ""];
        this.timer = 0;
        this.n_log = 5;
        this.i_log = 0;
        this.last = 1;
        this.first = this.last + this.n_log + 2;
        if (this.screenType == 2)
            term.clear();

        this.startup();
        this.started = 1;
    }

    startup() {
        if (this.screenType == 2) {
            term.moveTo(1, this.last).bgWhite.black.eraseLine();
            term(" -- log --");
            term.styleReset();
            this.started = 1;
            this.timer = setInterval(() => {
                var time = new Date();
                for (var i = 0; i < this.commands.length; i++) {
                    var cmd = this.commands[i];
                    if (cmd.screen.time) {
                        if (time - cmd.screen.time > 1000) {
                            term.moveTo(30, cmd.screen.index + this.first);
                            term(" ");
                            cmd.screen.time = 0;
                        }
                    }
                }
            }, 1000);
        }
    }

    shutdown() {
        if (this.screenType == 2) {
            this.started = 0;
            clearInterval(this.timer);
            term.clear();
        }
    }

    update(name, message) {
        if (!this.started)
            return;

        var index = this.index[name];

        if (index === undefined)
            return;

        var cmd = this.commands[index];

        if (cmd === undefined) {
            this.log("\t\tno topic " + name);
            return;
        }

        switch (this.screenType) {
            case 2:
                cmd.screen.time = Date.now();
                term.moveTo(1, cmd.screen.index + this.first);
                term(cmd.name);

                term.moveTo(30, cmd.screen.index + this.first);
                term("X");
                term.moveTo(32, cmd.screen.index + this.first);
                term(cmd.online == 1 ? "o" : cmd.online == 2 ? "s" : " ");
                term.moveTo(34, cmd.screen.index + this.first);
                if (cmd.type == 4) {
                    var obj = cmd.getValue();
                    var val = "";
                    for (const [name, command] of Object.entries(obj))
                        val += name + ","
                    term("" + val + "            ");
                } else
                    term("" + cmd.getValue(2) + " ");
                term.moveTo(50, cmd.screen.index + this.first);
                //term(new Date(el.update_time).toLocaleTimeString() + "  ")
                term(new Date(cmd.at).toLocaleTimeString() + "    ");
                if ('interval' in cmd) {
                    term.moveTo(70, cmd.screen.index + this.first);
                    term("" + cmd.interval + "  ")
                }
                break;
            case 3:
                var text = (cmd.online == 1 ? "o " : cmd.online == 2 ? "s " : "  ") + cmd.name + " ";
                text += cmd.getValue(2);
                text += new Date(cmd.at).toLocaleTimeString();
                if ('interval' in cmd)
                    text += " " + cmd.interval;
                console.log(text);
                break;
        }
    }

    log(text) {
        if (this.screenType == 2) {
            if (!this.started)
                return;
            term.white.bgBlack();
            this.logs[this.i_log] = (new Date()).toLocaleTimeString() + " " + text;
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

module.exports = Console