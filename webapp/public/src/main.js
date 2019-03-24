let devices = [];
let typeEnm = [];
let cmdEnm = [];

const deviceHandler = {
  reload: ()=>{
    devices.forEach(ui.update); 
  },
  update: (_upd)=>{
    let dev=devices.find(x=>x.id==_upd.id);
    //test device
    if(!dev){
      console.log('update device| ERR: not found',_upd);
      return;
    }
    dev=_upd;
    ui.update(dev);

  }
};

const ui = {
  binded: false,

  update: dev=>{
    const target=ui.getDevElem(dev.id);
    if(!target){
      console.log('ui.update | target not found',dev);
      return;
    }

    switch (dev.type) {

      // light
      case typeEnm.light: 
        if(dev.iVal==1)
          target.classList.add('on');
        else
          target.classList.remove('on');
        break;
    
      // enviroment device
      case typeEnm.temp:
      case typeEnm.humidity:
      case typeEnm.pressure:
        target.innerText = dev.fVal==0?'-':dev.fVal;
        break;
  
      // fan ////////////////////////////////////////
      case typeEnm.fan:

        //set target
        const targetBtn = target.children[dev.iVal];
        if(targetBtn.classList.contains('on')) return;

        //clear previosly selected
        const prev=target.querySelector(".on");
        if(prev)
          prev.classList.remove('on');

          //set on status
        targetBtn.classList.add('on');
        break;

      default:
        console.log('update unknown device|',dev);
        break;  
    }
  },

  bind: (elem)=>{
    
    // bind light
    if(elem.classList.contains("light")){
      elem.addEventListener("click", (ev)=>{
        socket.emit('command', {
          id: parseInt(ev.srcElement.attributes.dev.value),
          cmd: cmdEnm.CMD_SET,
          payload: ev.srcElement.classList.contains('on')?0:1
        })
      })
    }


  },

  fire: (devId, payload)=>{
    console.log('ui.fire |', devId);
    let dev=devices.find(x=>x.id==devId);
    socket.emit('command', {id: devId, cmd: cmdEnm.CMD_SET, 'payload': payload});
  },

  getDevElem: devId=>{
    return document.querySelector("[dev='"+devId+"']");
  }


};



(()=>{
  document.querySelectorAll("[dev]").forEach(ui.bind);
})();