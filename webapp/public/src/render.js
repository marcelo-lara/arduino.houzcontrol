"use strict";
const render={
    device: dev=>{
        const target=dev.elem;
        if(target===null) return;
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
              if(!dev.btn){
                console.log("not binded?");
                return;
              } 
              
              let rval="00000000"+dev.iVal.toString(2);
              rval=rval.substring(rval.length-8);
              for(let i=0; i<dev.btn.length; i++){
                const _btn=dev.btn[i].classList;
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
              switch(dev.status){
                case statusEnm.st_down: //connection down
                  status.st.className='err';
                  break;
                case statusEnm.st_offline: //node offline
                  status.st.className='offline';
                  break;
                case statusEnm.st_online: //node online
                  status.st.className='online';
                  break;
              }
      
              break;
              
            default:
              break;  
        }
    }
 
}