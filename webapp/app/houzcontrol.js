"use strict"
const enm = require('./enums');
const toHex = (val, len) => {
    return ("0000" + val.toString(16)).toUpperCase().substr(-len);
}

module.exports = {
    parse: (data, _serial) => {
        console.log('->', data);

        let matches = data.match(/\[(.{6,11})\]/);
        if (!matches) {
            return;
        }
        const _rawdev = matches[1];

        //server status
        if (_rawdev === 'online')
            return {act: enm.actEnm.action_conn, dev: module.exports.updateDevice(0, 1, enm.cmdEnm.CMD_VALUE)};
        if (_rawdev === 'offline')
            return {act: enm.actEnm.action_conn, dev: module.exports.updateDevice(0, 1, enm.cmdEnm.CMD_VALUE)};

        //split packet
        const _act = parseInt(_rawdev.substring(0, 1), 16);
        let updatePacket = {act: _act, dev: undefined};

        // debug
        console.log('::', enm.toStr(_act, enm.actEnm));
        
        if (_act !== enm.actEnm.action_rfReceived) return updatePacket; //udpate only on rfReceived
        const _node = parseInt(_rawdev.substring(2, 3), 16);
        const _cmd = parseInt(_rawdev.substring(4, 5), 16);
        const _dev = parseInt(_rawdev.substring(5, 7), 16);
        const _payload = parseInt(_rawdev.substring(7, 11), 16);
        updatePacket.dev = module.exports.updateDevice(_dev, _payload, _cmd, _serial);
        return updatePacket;
    },

    //update device
    updateDevice: (_id, payload, cmd, _serial) => {
        const dev = module.exports.devices.find(x => x.id === _id);
        if (!dev) { console.log('\tdevice not found??\t', _id); return; };
        
        console.log("\t"+ dev.name)
        switch (dev.type) {
            case enm.typeEnm.node:
                dev.iVal = payload; //store status

                //handle announce
                if (dev.id === 0 || cmd != enm.cmdEnm.CMD_STATUS) return;
                console.log("\t> node announce..");
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
                dev.fVal = (payload / 100) + 900;
                console.log("\tpressure:", dev.fVal, "hPa");
                break;

            case enm.typeEnm.array2x:
            case enm.typeEnm.array4x:
            case enm.typeEnm.array8x:
                dev.iVal = payload;
                console.log("\tarray:", dev.iVal, "B");
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
        return pkt;
    },

    devices: [
        {id: 0x00, name: 'server_node', type: enm.typeEnm.node, fVal: 0, iVal: 0, status: -1},

        // Office
        {name: 'office_node',       id:  0x1, type: enm.typeEnm.node, node: 1}, //N1DC04F0F0
        {name: 'office_AC',         id: 0x11, type: enm.typeEnm.ac, node: 1}, //Air Conditioner on/off
        {name: 'office_light',      id: 0x13, type: enm.typeEnm.light, node: 1}, //ceiling light | N1DC130001
        {name: 'external_light',    id: 0x17, type: enm.typeEnm.envlight, node: 1}, //light sensor [0-1024]
        {name: 'external_temp',     id: 0x1A, type: enm.typeEnm.temp, node: 1}, //temperature [celsius /100] | N1DA1A0000
        {name: 'external_humidity', id: 0x1B, type: enm.typeEnm.humidity, node: 1}, //humidity [%] | N1DA1B0000
        {name: 'external_pressure', id: 0x1C, type: enm.typeEnm.pressure, node: 1}, //pressure [hPa /FIX THIS ((x-900) * 100?)] | N1DA1C0000
        {name: 'external_weather',  id: 0x1F, type: enm.typeEnm.enviroment, node: 1}, //all devices | N1DA1F0099

        //suite devices
        {name: 'suite_node',		id: 0x2,  type: enm.typeEnm.node,      node: 2},
        {name: 'suite_light',		id: 0x21, type: enm.typeEnm.light,     node: 2},
        {name: 'suite_fan',			id: 0x22, type: enm.typeEnm.fan,       node: 2},
        {name: 'suite_AC',			id: 0x23, type: enm.typeEnm.ac,        node: 2},
        {name: 'suite_enviroment',	id: 0x25, type: enm.typeEnm.enviroment,node: 2},
        {name: 'suite_temp',		id: 0x26, type: enm.typeEnm.temp,      node: 2},
        {name: 'suite_humidity',	id: 0x27, type: enm.typeEnm.humidity,  node: 2},
        {name: 'suite_pressure',	id: 0x28, type: enm.typeEnm.pressure,  node: 2},
		
		//living devices
        {name: 'living_node',       id:  0x3, type: enm.typeEnm.node,    node: 3},
        {name: 'living_mainLight',  id: 0x33, type: enm.typeEnm.array2x, node: 3},  
        {name: 'living_dicroLight', id: 0x34, type: enm.typeEnm.array8x, node: 3},  
        {name: 'living_spotLight',  id: 0x35, type: enm.typeEnm.array4x, node: 3},  
        {name: 'living_fxLight',    id: 0x36, type: enm.typeEnm.array2x, node: 3},  
        {name: 'living_fx',         id: 0x37, type: enm.typeEnm.fxanim,  node: 3},  
        {name: 'living_AC',         id: 0x38, type: enm.typeEnm.ac,      node: 3}
    ]
}

