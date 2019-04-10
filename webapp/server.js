"use strict";
const http = require('http')
const express = require('express')
const app = express()

const serial = require('serialport')
const sp_readline = serial.parsers.Readline
const enums = require('./app/enums');
const houzcontrol = require('./app/houzcontrol');
const portName="com7"; //"/dev/ttyUSB0"; //com7

const port = 3000
const Server = http.createServer(app)
const io = require('socket.io').listen(Server)
app.use(express.static(__dirname + '/public'))

// debug
serial.list((err, ports)=>{
  console.log('--SERIAL--');
  ports.forEach(port=>{
    console.log('>', port);
  });
});


// serial port connection
const serialPort = new serial(portName, {baudRate: 115200});
const parser = new sp_readline()

serialPort.pipe(parser);
serialPort.on('open', () => {
  console.log('..Serial Port Opened')
})

parser.on('data', data => {
  let dev = houzcontrol.parse(data, serialPort);
  if(dev)  
    io.emit('update', dev)
});

// clients handling
io.on('connection', socket => {
  console.log('io.connect | ', socket.id);

  socket.on('disconnect', socket=>{
    console.log('io.disconnect | ', socket.id);
  });  

  socket.on('command', data=>{
    console.log('io.cmd | ', socket.id,'>',data);

    const pkt = houzcontrol.encodePacket(data);

    if(pkt){
      console.log('<- ',pkt);
      serialPort.write(pkt+"\n");
    }
      
  });

  // deliver device list to client
  socket.emit('data', {
    'devices': houzcontrol.devices,
    'typeEnm': enums.typeEnm,
    'cmdEnm': enums.cmdEnm
  });
});

Server.listen(port, () => {
  console.log(`Express server started on ${port}`)
})

