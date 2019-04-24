"use strict";
const bind={
    all: ()=>{
        devices.forEach(dev=>{
            if(!dev.elem){
                dev.elem=document.querySelector('div[dev="'+dev.id+'"]');

                if(bind.binded.indexOf(dev.id)<0){
                    bind.binded.push(dev.id);

                    if(dev.elem!==null){
                        switch(dev.type){
                  
                            //lights
                            case typeEnm.array2x: bind.lightArray(dev, 2); break;
                            case typeEnm.array4x: bind.lightArray(dev, 4); break;
                            case typeEnm.array8x: bind.lightArray(dev, 8); break;
                            case typeEnm.light:   bind.light(dev); break;
                            case typeEnm.fxanim:  bind.fxanim(dev); break;
    
                            //weather
                            case typeEnm.enviroment: bind.weather(dev); break;
    
                            //fan
                            case typeEnm.fan: bind.fan(dev); break;
    
                            //ac
    
                            //node
    
                            default: break;
                        }
                    }
    

                };
            }
        }); 
    },

/* ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */
    lightArray: (dev, alen)=>{

        //add buttons
        const _btn = dev.elem.querySelectorAll("div[b]");
        if(_btn.length<1){
            //default bind
            dev.btn = dev.elem.children;
        }else{
            //custom bind
            dev.btn=[];
            for(let b=0; b<alen; b++){dev.btn.push(dev.elem.querySelector('div[b="'+b+'"]'));}
        }

        //bind buttons
        for(let b=0; b<alen; b++){
            dev.btn[b].addEventListener("click", (ev)=>{
                
                const bitval=Math.pow(2, b);
                
                //determine status
                let newVal=dev.iVal;
                if(ev.target.classList.contains('on'))
                    newVal -= bitval;
                else
                    newVal += bitval;

                //send packet
                socket.emit('command', {'id': dev.id, 'cmd': cmdEnm.CMD_SET, 'payload': newVal});

            },false);
        }

    },

    light: dev=>{
        dev.elem.addEventListener("click", (ev)=>{
            socket.emit('command', {
                id: dev.id,
                cmd: cmdEnm.CMD_SET,
                payload: dev.elem.classList.contains('on')?0:1
            });
        });
    },

/* ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */
    weather: dev=>{
        dev.elem.addEventListener("click", ()=>{
            socket.emit('command', {
              id: dev.id,
              cmd: cmdEnm.CMD_QUERY
            })
        })    
    },
    
    fan: dev=>{
        for(let i=0; i<dev.elem.children.length; i++){
            let pkt={
              id: dev.id,
              cmd: cmdEnm.CMD_SET,
              payload: i
            };
            dev.elem.children[i].addEventListener("click", ()=>{
              socket.emit('command', pkt)
            });
        }        
    },
    
    fxanim: dev=>{
        for(let i=0; i<dev.elem.children.length; i++){
            let pkt={
              id: dev.id,
              cmd: cmdEnm.CMD_SET,
              payload: parseInt(dev.elem.children[i].attributes.val.value,16)
            };
            dev.elem.children[i].addEventListener("click", ()=>{
              socket.emit('command', pkt)
            });
        }        
    },

    scene: (panel)=>{
        if(!panel)
            return;
        panel.addEventListener("click", (ev)=>{
            
            console.log("panel click", ev.target);
            socket.emit('command', {
                id: 0,
                cmd: cmdEnm.CMD_SET,
                payload: parseInt(ev.target.getAttribute("val"))
            });

        });
            
    },

    binded: []

}
