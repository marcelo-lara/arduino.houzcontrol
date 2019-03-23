const socket = io.connect('http://localhost:3000')

socket.on('connected', () => {
  console.log('Socket Connected')
})
socket.on('disconnect', () => {
  console.log('Socket Disconnected')
})

socket.on('data', data => {
  console.log('>>', data);
})

document.getElementById("cbBtn").addEventListener('click', ev=>{
  console.log('<<try send');
  socket.emit('command', {tst: 'wow!'});

});