module.exports = {
    parse: (data)=>
    {
        let matches = data.match(/\[(.*?)\]/);
        if (!matches) return;

        console.log('--houz.parse', data);
        const _rawdev = matches[1];

        //server status
        if(_rawdev==='online')
            return module.exports.updateDevice(0, 1);
        
    
        //split packet
        const _act = _rawdev.substring(0, 1);

        if(_act!=='4') return; //udpate only on rfReceived
        const _node=_rawdev.substring(2, 3);
        const _cmd=_rawdev.substring(4, 5);
        const _dev=_rawdev.substring(5, 7);
        const _payload=_rawdev.substring(7, 11);
    
        return module.exports.updateDevice(parseInt(_dev,16), parseInt(_payload,16));

        // debug
        console.log('\t--', module.exports.actToStr(_act));
        console.log('\tnode\t', _node);
        console.log('\tdev\t', _dev, parseInt(_dev,16));
        console.log('\tcmd\t', _cmd);
        console.log('\tpayload\t', _payload);


    },
    actToStr: (act)=>{
        switch (act) {
          case '0': return 'log'; break;
          case '1': return 'rfSentOk'; break;
          case '2': return 'rfSentRetry'; break;
          case '3': return 'rfSentFail'; break;
          case '4': return 'rfReceived'; break;
          default:
            break;
        }
    },

    //update device
    updateDevice: (_id, payload)=>{
        const dev=module.exports.devices.find(x=>x.id===_id);
        if(!dev){
            console.log('\tdevice not found\t', id);
            return;
        };

        switch (dev.devType) {
            case 0: //node
            case 1: //light
                dev.iVal=(payload==1?1:0);
                break;
            default:
                console.log('\tdevice: not defined');
                dev.iVal=payload;
                dev.fVal=parseFloat(payload);
                break;
        }
        dev.lastUpdate=new Date();
        return dev;
    },
    devices: [
        {id: 0x00, name: 'server_node', devType: 0, fVal:0, iVal:0},
        {id: 0x21, name: 'suite_light', devType: 1, fVal:0, iVal:0}
    ]

}