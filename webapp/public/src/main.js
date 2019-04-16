let devices = [];
let typeEnm = [];
let cmdEnm = [];
let atcEnm = [];
let statusEnm = [];


const deviceHandler = {
  bind: ()=>{
    bind.allDevices();

    devices.forEach(ui.update); 
  },
  update: (_upd)=>{

    //status lights
    ui.status(_upd.act);

    //check if device exists
    if(!_upd.dev) return;
    let dev=devices.find(x=>x.id==_upd.dev.id);
    if(!dev) return;
    
    //update data
    dev.iVal=_upd.dev.iVal;
    dev.fVal=_upd.dev.fVal;

    //render device
    ui.update(dev);
  }
};

const ui = {
  binded: false,
  statusRx: undefined,
  statusTx: undefined,
  statusSt: undefined,
  _statusShow: (elem, status, timeout)=>{
    if(!timeout) timeout=200;
    elem.className=status;
    setTimeout(()=>{elem.className='';},timeout);
  },
  status: stat=>{
    switch(stat){
      case actEnm.action_rfReceived:
        ui._statusShow(ui.statusRx, 'rfOk');
        break;
        case actEnm.action_rfSentOk:
        ui._statusShow(ui.statusTx, 'rfOk');
        break;
        case actEnm.action_rfSentFail:
        ui._statusShow(ui.statusTx, 'rfFail', 2000);
        break;
        case actEnm.action_rfSentRetry:
        ui._statusShow(ui.statusTx, 'rfRetry');
        break;
    }
  },

  update: dev=>{
    const target=ui.getDevElem(dev.id);
    if(!target){
      console.log('ui.update | target not found',dev);
      return;
    }
    if(dev.iVal===undefined) dev.iVal=0;
    if(dev.fVal===undefined) dev.fVal=0;

    switch (dev.type) {

      // light
      case typeEnm.light: 
        if(dev.iVal==1)
          target.classList.add('on');
        else
          target.classList.remove('on');
        break;
    
      case typeEnm.array2x:
      case typeEnm.array4x:
      case typeEnm.array8x:
        // console.log(dev);
        if(!dev.btn){
          // console.log("not binded?");
          return;
        } 
        
        let rval="00000000"+dev.iVal.toString(2);
        rval=rval.substring(rval.length-8);
        // console.log(rval);
        for(i=0; i<dev.btn.length; i++){
          _btn=dev.btn[i].classList;
          if(rval[7-i]==="1")
          {
            if(!_btn.contains('on')) _btn.add('on');
          }else{
            if(_btn.contains('on')) _btn.remove('on');
          }
        }
        break;

      // enviroment device /////////////////////////
      case typeEnm.temp:
      case typeEnm.humidity:
      case typeEnm.pressure:
        target.innerText = dev.fVal==0?'-':dev.fVal.toFixed(2);
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

      // node ////////////////////////////////////////
      case typeEnm.node:
        // console.log('update node',dev);
        switch(dev.status){
          case statusEnm.st_down: //connection down
            ui.statusSt.className='err';
            break;
          case statusEnm.st_offline: //node offline
            ui.statusSt.className='offline';
            break;
          case statusEnm.st_online: //node online
            ui.statusSt.className='online';
            break;
        }

        break;
        
      default:
        // console.log('update unknown device|',dev);
        break;  
    }
  },

  bind: (dev)=>{
    const elem=ui.getDevElem(dev.id);
    if(!elem){
      //console.log('ui.bind | element not found',dev);
      return;
    }
    if(elem.getAttribute("bind")!=null){
      //console.log('ui.bind | already binded..',dev);
      return;
    }
    elem.setAttribute("bind", "1")
    
    // bind wheater //////////////////////////////////////////////
    if(elem.classList.contains("weather")){
      elem.addEventListener("click", (ev)=>{
        socket.emit('command', {
          id: parseInt(elem.attributes.dev.value),
          cmd: cmdEnm.CMD_QUERY
        })
      })
    }

    switch (dev.type) {
      // bind lights //////////////////////////////////////////////
        case typeEnm.light:
          elem.addEventListener("click", (ev)=>{
            socket.emit('command', {
              id: parseInt(elem.attributes.dev.value),
              cmd: cmdEnm.CMD_SET,
              payload: elem.classList.contains('on')?0:1
            })
          })
        break;
      
      // bind fan //////////////////////////////////////////////
      case typeEnm.fan:
          for(i=0; i<elem.children.length; i++){
            let pkt={
              id: parseInt(elem.attributes.dev.value),
              cmd: cmdEnm.CMD_SET,
              payload: i
            };
            elem.children[i].addEventListener("click", ()=>{
              socket.emit('command', pkt)
            });
          }        
          break;
      case typeEnm.fxanim:
      for(i=0; i<elem.children.length; i++){
        let pkt={
          id: parseInt(elem.attributes.dev.value),
          cmd: cmdEnm.CMD_SET,
          payload: parseInt(elem.children[i].attributes.val.value,16)
        };
        elem.children[i].addEventListener("click", ()=>{
          socket.emit('command', pkt)
        });
      }        
    break;

      default:
        break;
    }

  },
  _bindArray: dev=>{

  },

  fire: (devId, payload)=>{
    // console.log('ui.fire |', devId);
    let dev=devices.find(x=>x.id==devId);
    socket.emit('command', {id: devId, cmd: cmdEnm.CMD_SET, 'payload': payload});
  },

  getDevElem: devId=>{
    return document.querySelector("[dev='"+devId+"']");
  }


};



(()=>{

  //server status
  ui.statusSt=document.getElementById('st');
  ui.statusRx=document.getElementById('rx');
  ui.statusTx=document.getElementById('tx');
  
  //document.querySelectorAll("[dev]").forEach(ui.bind);
})();