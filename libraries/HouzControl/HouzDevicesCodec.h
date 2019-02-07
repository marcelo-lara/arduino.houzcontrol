#include <HouzDevicesModel.h>

class HouzDevicesCodec{
    public:
        unsigned long encode(u8 _cmd, u8 deviceId, u32 devicePayload);
    	deviceData decode(u32 rawData, u32 rfNodeStation);
        deviceData decode(String str);

        unsigned long StrToHex(char str[]);
        unsigned long StrToHex(String str);
        u32 pressureEncode(u32 devicePayload);
        u32 pressureDecode(u32 devicePayload);
        
    private:
};