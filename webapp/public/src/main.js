let devices = [];
let typeEnm = [];
let cmdEnm = [];
let atcEnm = [];
let statusEnm = [];

const deviceHandler = {
  bind: ()=>{
    bind.all();
    devices.forEach(render.device); 
  },

  update: (_upd)=>{

    //status lights
    status.render(_upd.act);

    //check if device exists
    if(!_upd.dev) return;
    let dev=devices.find(x=>x.id==_upd.dev.id);
    if(!dev) return;
    
    //update data
    dev.iVal=_upd.dev.iVal;
    dev.fVal=_upd.dev.fVal;

    //render device
    render.device(dev);
  }
};

const status = {
  rx: undefined,
  tx: undefined,
  st: undefined,
  setup: ()=>{
    status.st=document.getElementById('st');
    status.rx=document.getElementById('rx');
    status.tx=document.getElementById('tx');
  },
  _render: (elem, status, timeout)=>{
    if(!timeout) timeout=200;
    elem.className=status;
    setTimeout(()=>{elem.className='';},timeout);
  },
  render: stat=>{
    switch(stat){
      case actEnm.action_rfReceived:  status._render(status.rx, 'rfOk'); break;
      case actEnm.action_rfSentOk:    status._render(status.tx, 'rfOk'); break;
      case actEnm.action_rfSentFail: status._render(status.tx, 'rfFail', 2000); break;
      case actEnm.action_rfSentRetry: status._render(status.tx, 'rfRetry'); break;
    }
  }
};



(()=>{
  status.setup();
})();