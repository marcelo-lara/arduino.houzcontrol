"use strict"
const enm = require('./enums');
const toHex = (val, len) => {
    return ("0000" + val.toString(16)).toUpperCase().substr(-len);
}

module.exports = {
    parse: (data, _serial) => {
        let matches = data.match(/\[(.{6,11})\]/);
        if (!matches) {
            //console.log('--houz.parse ERR |', data);
            return;
        }
        const _rawdev = matches[1];

        //server status
        if (_rawdev === 'online')
            return module.exports.updateDevice(0, 1, enm.cmdEnm.CMD_VALUE);
        if (_rawdev === 'offline')
            return module.exports.updateDevice(0, 0, enm.cmdEnm.CMD_VALUE);

        //split packet
        const _act = parseInt(_rawdev.substring(0, 1), 16);

        if (_act !== enm.actEnm.action_rfReceived) return; //udpate only on rfReceived
        const _node = parseInt(_rawdev.substring(2, 3), 16);
        const _cmd = parseInt(_rawdev.substring(4, 5), 16);
        const _dev = parseInt(_rawdev.substring(5, 7), 16);
        const _payload = parseInt(_rawdev.substring(7, 11), 16);
        return module.exports.updateDevice(_dev, _payload, _cmd, _serial);

        // debug
        console.log('\t--', module.exports.actToStr(_act));
        console.log('\tnode\t', _node);
        console.log('\tdev\t', _dev, parseInt(_dev, 16));
        console.log('\tcmd\t', _cmd);
        console.log('\tpayload\t', _payload);
    },

    //update device
    updateDevice: (_id, payload, cmd, _serial) => {
        const dev = module.exports.devices.find(x => x.id === _id);
        if (!dev) { console.log('\tdevice not found??\t', _id); return; };
        
        console.log(dev.name)
        switch (dev.type) {
            case enm.typeEnm.node:
                dev.iVal = payload; //store status

                //handle announce
                if (dev.id === 0 || cmd != enm.cmdEnm.CMD_STATUS) return;
                console.log("\t> announce..");
                _serial.write("N" + toHex(dev.node, 1) + "DA0" + toHex(dev.node, 1) + "0000\n");
                break;

            case enm.typeEnm.light:
                dev.iVal = (payload == 1 ? 1 : 0);
                console.log("\tlight", dev.iVal == 1 ? 'on' : 'off');
                break;

            case enm.typeEnm.fan:
                dev.iVal = payload;
                console.log("\tfan", dev.iVal);
                break;

            case enm.typeEnm.temp:
                dev.fVal = payload / 100;
                console.log("\ttemp:", dev.fVal, "º");
                break;
            case enm.typeEnm.humidity:
                dev.fVal = payload / 100;
                console.log("\thumidity:", dev.fVal, "%");
                break;
            case enm.typeEnm.pressure:
                dev.fVal = payload / 100;
                console.log("\pressure:", dev.fVal, "hPa");
                break;

            default:
                console.log('\tdeviceType not handled', enm.toStr(dev.type, enm.typeEnm));
                dev.iVal = payload;
                dev.fVal = parseFloat(payload);
                break;
        }
        dev.lastUpdate = new Date();
        return dev;
    },
    encodePacket: (_raw) => {
        if (!_raw.id) { console.log('\tencodePacket: ERR id not defined .. abort'); return; }
        console.log('--encodePacket |', _raw);

        //set target device
        const dev = module.exports.devices.find(x => x.id == _raw.id)
        if (!dev) { console.log('\tencodePacket: ERR device not found .. abort'); return; }

        //set defaults
        const cmd = _raw.cmd ? _raw.cmd : enm.cmdEnm.CMD_QUERY;
        const payload = _raw.payload ? _raw.payload : 0;

        //build packet
        let pkt = "N" + dev.node + "D";
        pkt += toHex(cmd, 1);
        pkt += toHex(dev.id, 2);
        pkt += toHex(payload, 4);
        console.log('\tpacket |', pkt);

        return pkt;
    },

    devices: [
        { id: 0x00, name: 'server_node', type: enm.typeEnm.node, fVal: 0, iVal: 0 },

        //suite devices
        { id: 0x2,  node: 2, name: 'suite_node',        type: enm.typeEnm.node,     fVal: 0, iVal: 0 },
        { id: 0x21, node: 2, name: 'suite_light',       type: enm.typeEnm.light,    fVal: 0, iVal: 0 },
        { id: 0x22, node: 2, name: 'suite_fan',         type: enm.typeEnm.fan,      fVal: 0, iVal: 0 },
        { id: 0x23, node: 2, name: 'suite_AC',          type: enm.typeEnm.ac,       fVal: 0, iVal: 0 },
        { id: 0x25, node: 2, name: 'suite_enviroment',  type: enm.typeEnm.enviroment, fVal: 0, iVal: 0 },
        { id: 0x26, node: 2, name: 'suite_temp',        type: enm.typeEnm.temp,     fVal: 0, iVal: 0 },
        { id: 0x27, node: 2, name: 'suite_humidity',    type: enm.typeEnm.humidity, fVal: 0, iVal: 0 },
        { id: 0x28, node: 2, name: 'suite_pressure',    type: enm.typeEnm.pressure, fVal: 0, iVal: 0 },

    ]

}

