const socket = io.connect(window.location.href);

socket.on('connected', () => {
  console.log('Socket Connected')
})
socket.on('disconnect', () => {
  console.log('Socket Disconnected')
})

socket.on('data', data => {
  console.log('data>>', data);
  if(data.devices) devices=data.devices;
  if(data.typeEnm) typeEnm=data.typeEnm;
  if(data.cmdEnm) cmdEnm=data.cmdEnm;
  if(data.actEnm) actEnm=data.actEnm;
  if(data.statusEnm) statusEnm=data.statusEnm;
 
  deviceHandler.reload();
});

socket.on('update', deviceHandler.update) //delegate update handling