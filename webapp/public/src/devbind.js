const devBind={
    lightArray: (dev, alen)=>{

        if(dev.elem) return;
        // socket.emit("debug", "devBind.lightArray "+alen);
        dev.elem=devBind._getElem(dev);
        if(!dev.elem) return;

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
    _getElem: dev=>{
        return document.querySelector('div[dev="'+dev.id+'"]');
    }
}
