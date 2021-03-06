const Command = require("./command.js");
const Automation = require("./automation.js");

var commands = [
    new Command('water/1/level', 2, true),
    new Command('water/2/level', 2, true),
    new Command('water/valve', 1, true, Automation.prototype.normalDigital, { GPO: [4], safe: 0 }),
    new Command('water/pump', 1, true, Automation.prototype.normalDigital, { GPO: [17], safe: 0 }),
    new Command('water/1/temp', 2, true),
    new Command('water/2/temp', 2, true),
    new Command('water/1/unload', 1, true, Automation.prototype.normalDigital, { GPO: [27], safe: 0 }),
    new Command('water/2/unload', 1, true, Automation.prototype.normalDigital, { GPO: [22], safe: 0 }),
    new Command('sensor/int/temp', 2, true),
    new Command('sensor/int/hum', 2, true),
    new Command('sensor/ext/temp', 2, true),
    new Command('sensor/ext/gas/1', 2, true),
    new Command('sensor/int/gas/1', 2, true),
    new Command('sensor/ext/hum', 2, true),
    new Command('battery/1/volt', 2, true),
    new Command('battery/1/amp', 2, true),
    new Command('battery/1/back', 2, true),
    new Command('battery/2/volt', 2, true),
    new Command('battery/2/amp', 2, true),
    new Command('battery/3/volt', 2, true),
    new Command('battery/3/amp', 2, true),
    new Command('solar/volt', 2, true),
    new Command('solar/amp', 2, true),
    new Command('gas/1/level', 2, true),
    new Command('gas/2/level', 2, true),
    new Command('lights', 1, true, Automation.prototype.normalDigital, { GPO: [5], safe: 0 }),
    new Command('light/extern', 1, true, null, { safe: 0 }),
    new Command('light/living', 1, true, null, { safe: 0 }),
    new Command('light/kitchen', 1, true, null, { safe: 0 }),
    new Command('generale', 1, true, Automation.prototype.normalDigital, { GPO: [6], safe: 0 }),
    new Command('inverter', 1, true, Automation.prototype.normalDigital, { GPO: [13], safe: 0 }),
    new Command('fridge/on', 1, true, null, { safe: 0 }),
    new Command('heater/on', 1, true, null, { safe: 0 }),
    new Command('airc/on', 1, true, null, { safe: 0 }),
    new Command('fans/1', 1, true, Automation.prototype.normalDigital, { GPO: [19], safe: 0 }),
    new Command('fans/2', 1, true, Automation.prototype.normalDigital, { GPO: [26], safe: 0 }),
    new Command('fans/3', 1, true, Automation.prototype.normalDigital, { GPO: [18], safe: 0 }),
    new Command('fans/4', 1, true, Automation.prototype.normalDigital, { GPO: [23], safe: 0 }),
    new Command('gas/wakeup', 3, true, false, { safe: "" }),
    new Command('slave/url/open', 3, false, Automation.prototype.addSlave, { safe: "" }),
    new Command('slave/url/close', 3, false, Automation.prototype.removeSlave, { safe: "" }),
    new Command("slaves", 4, true, null, { safe: {} })
]

module.exports = commands