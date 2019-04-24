"use strict";
const http = require('http')
const express = require('express')
const app = express()

const serial = require('serialport')
const sp_readline = serial.parsers.Readline
const enums = require('./app/enums');
const houzcontrol = require('./app/houzcontrol');
const portName = process.env.PORT; //"/dev/ttyUSB0"; //com7

const port = 3000
const Server = http.createServer(app)
const io = require('socket.io').listen(Server)
app.use(express.static(__dirname + '/public'))

// serial port connection
const serialPort = new serial(portName, {baudRate: 115200});
const parser = new sp_readline()

serialPort.pipe(parser);
serialPort.on('open', () => {
  console.log('link on', portName);
  io.emit('update', 
  {
    act: enums.actEnm.action_ack,
    dev: houzcontrol.updateDevice(0,1,enums.statusEnm.st_offline,serialPort)
  });
})

parser.on('data', data => {
  let pkt = houzcontrol.parse(data, serialPort);
  if(pkt)  
    io.emit('update', pkt)
});

// clients handling
io.on('connection', socket => {
  //console.log('io.connect | ', socket.id);

  socket.on('disconnect', socket=>{
    //console.log('io.disconnect | ', socket.id);
  });  

  socket.on('command', data=>{
    console.log('io.cmd < ', socket.id,'|',data);
    const pkt = houzcontrol.encodePacket(data);
    if(pkt){
      console.log('<- ',pkt);
      serialPort.write(pkt+"\n");
    }
  });
  socket.on('debug', data=>{
    console.log('io.debug | ', socket.id,'>',data);
  });
  
  // deliver device list to client
  socket.emit('data', {
    'devices': houzcontrol.devices,
    'typeEnm': enums.typeEnm,
    'cmdEnm': enums.cmdEnm,
    'actEnm': enums.actEnm,
    'statusEnm': enums.statusEnm
  });
});

Server.listen(port, () => {
  console.log(`Server started on ${port}`)
})

