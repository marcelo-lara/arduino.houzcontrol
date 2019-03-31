module.exports = {
    typeEnm: {
        node: 0,
        light: 1,
        ac: 2,
        fan: 3,
        enviroment: 4,
        temp: 5,
        humidity: 6,
        pressure: 7,
        array2x: 8,
        array4x: 9,
        array8x: 10,
        envlight: 11,
        fxanim: 12
    },
    actEnm: {
        action_log			: 0x00,
        action_rfSentOk		: 0x01,
        action_rfSentRetry	: 0x02,
        action_rfSentFail	: 0x03,
        action_rfReceived	: 0x04,
        action_irReceived	: 0x10
    },
    cmdEnm: {
        CMD_QUERY   :0xA,
        CMD_VALUE   :0xB,
        CMD_SET	    :0xC,
        CMD_EVENT   :0xD,
        CMD_STATUS  :0xE
    },
    
    //enum to string helper
    toStr: (val, enm)=>{
        try {return Object.entries(enm).find(x=>x[1]===val)[0];} 
        catch (error) {return "";}}
}