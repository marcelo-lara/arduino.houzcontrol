"use strict";
const http = require('http')
const express = require('express')
const app = express()
const serialport = require('serialport')
const sp_readline = serialport.parsers.Readline
const houzcontrol = require('./app/houzcontrol');

const port = 3000
const Server = http.createServer(app)
const io = require('socket.io').listen(Server)

app.use(express.static(__dirname + '/public'))

var devices=new Array();

// debug
serialport.list((err, ports)=>{
  ports.forEach(port=>{
    console.log('port', port.comName, port);
  });
});

// serial port connection
const serialPort = new serialport('COM7', {baudRate: 115200})
const parser = new sp_readline()

serialPort.pipe(parser);
serialPort.on('open', () => {
  console.log('..Serial Port Opened')
})

parser.on('data', data => {
  let dev = houzcontrol.parse(data);
  if(dev)  
    io.emit('data', dev)
});

// clients handling
io.on('connection', socket => {
  console.log('io ', socket.id,'| connected',);

  socket.on('disconnect', socket=>{
    console.log('io ', socket.id,'| disconnected',);
  });  

  socket.on('command', data=>{
    console.log('io ', socket.id,' msg | ',data);
  });

  // deliver device list to client
  socket.emit('data', houzcontrol.devices);
});


Server.listen(port, () => {
  console.log(`Express server started on ${port}`)
})

class Device {
  constructor(id, name) {
    this.id = id;
    this.name = name;
    this.lastUpdate = new Date();
    this.iVal=0;
    this.fVal=0;
    this.devType=0;
    this.devNode = 0;
  }
}
