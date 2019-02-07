#include "Arduino.h"
#include "HouzDevicesCodec.h"
#include "HouzDevicesModel.h"


unsigned long HouzDevicesCodec::encode(u8 _cmd, u8 deviceId, u32 devicePayload)
{
  devicePayload=pressureEncode(deviceId, devicePayload);

	unsigned long retVal = 0xD;
	retVal = (retVal << 4) + _cmd;
	retVal = (retVal << 8) + deviceId;
	retVal = (retVal << 16) + devicePayload;
	return retVal;
}
deviceData HouzDevicesCodec::decode(u32 rawData, u32 nodeId) {
	deviceData decoded;
	decoded.raw = rawData;
	decoded.hasData = false;

	if (((rawData >> 28) == 0xD)) {
		decoded.node = nodeId;

		//parse values
		decoded.hasData = true;
		decoded.cmd = ((rawData >> 24) & 0x0F);
		decoded.id = ((rawData >> 16) & 0x0FF);
		decoded.payload = ((rawData) & 0x0000FFFF);
	}
  decoded.payload = pressureDecode(decoded.id, decoded.payload);
	return decoded;
};

deviceData HouzDevicesCodec::decode(String str) { //from serial
	deviceData dev;
	if (str.length() != 10 || str[0] != 'N' || str[2] != 'D')
		return dev;

	dev.node = StrToHex(str.substring(1, 2));
	dev.cmd = StrToHex(str.substring(3, 4));
	dev.id = StrToHex(str.substring(4, 6));
	dev.payload = StrToHex(str.substring(6, 10));
	dev.raw = StrToHex(str.substring(3, 10));
	dev.hasData = (dev.id != 0);
	return dev;
}

//////////////////////////////////////////////////////
// Pressure patch
u32 HouzDevicesCodec::pressureEncode(u8 deviceId, u32 devicePayload){
	if (deviceId!=0x27) return devicePayload;
  return (devicePayload-850)*100; //800hPa offset
}

u32 HouzDevicesCodec::pressureDecode(u8 deviceId, u32 devicePayload){
	if (deviceId!=0x27) return devicePayload;
  return (devicePayload/100)+850; //800hPa offset
}


//////////////////////////////////////////////////////
// StrToHex
unsigned long HouzDevicesCodec::StrToHex(char str[])
{
  return (long)strtol(str, 0, 16);
};
unsigned long HouzDevicesCodec::StrToHex(String str)
{
  return (long)strtol(str.c_str(), 0, 16);
};
