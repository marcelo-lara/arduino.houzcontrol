let devices = [];
let typeEnm = [];
let cmdEnm = [];
let atcEnm = [];
let statusEnm = [];


const deviceHandler = {
  reload: ()=>{
    devices.forEach(ui.update); 
    if(!ui.binded)
      devices.forEach(ui.bind); 
  },
  update: (_upd)=>{
    console.log(_upd);
    ui.status(_upd.act);
    if(!_upd.dev) return;
    let dev=devices.find(x=>x.id==_upd.dev.id);
    //test device
    if(!dev){
      console.log('update device| ERR: not found',_upd);
      return;
    }
    ui.update(_upd.dev);
  }
};

const ui = {
  binded: false,
  statusRx: undefined,
  statusTx: undefined,
  statusSt: undefined,
  statusShow: (elem, status, timeout)=>{
    if(!timeout) timeout=200;
    elem.className=status;
    setTimeout(()=>{elem.className='';},timeout);
  },
  status: stat=>{
    switch(stat){
      case actEnm.action_rfReceived:
        ui.statusShow(ui.statusRx, 'rfOk');
        break;
        case actEnm.action_rfSentOk:
        ui.statusShow(ui.statusTx, 'rfOk');
        break;
        case actEnm.action_rfSentFail:
        ui.statusShow(ui.statusTx, 'rfFail', 2000);
        break;
        case actEnm.action_rfSentRetry:
        ui.statusShow(ui.statusTx, 'rfRetry');
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
        let rval="00000000"+dev.iVal.toString(2);
        rval=rval.substring(rval.length-8);
        target.setAttribute('data',rval);
        for(i=0; i<target.children.length; i++){
          targetChild=target.children[i].classList;
          if(rval[7-i]==="1")
          {
            if(!targetChild.contains('on')) targetChild.add('on');
          }else{
            if(targetChild.contains('on')) targetChild.remove('on');
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
        console.log('update node',dev);
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
        console.log('update unknown device|',dev);
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

        case typeEnm.array2x:
        case typeEnm.array4x:
        case typeEnm.array8x:
          for(i=0; i<elem.children.length; i++){
            let bit=Math.pow(2, i);
            let pkt={id: dev.id, cmd: cmdEnm.CMD_SET, payload: (i+1)};
            elem.children[i].addEventListener("click", (ev)=>{
              const srcElement = ev.srcElement;
              let currVal = parseInt( ev.srcElement.parentElement.attributes.data.value,2);
              if(srcElement.classList.contains('on'))
                currVal -= bit;
              else
                currVal += bit;
              pkt.payload=currVal;
              socket.emit('command', pkt)
            });
          }
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
  ui.statusSt=document.getElementById('st');
  ui.statusRx=document.getElementById('rx');
  ui.statusTx=document.getElementById('tx');
  
  //document.querySelectorAll("[dev]").forEach(ui.bind);
})();