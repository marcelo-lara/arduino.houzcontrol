/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Devices Config

// Master
#define server_node			0    //N0DA000000 ping?

// Office
#define office_node			0x1  //N1DC04F0F0
#define office_AC			0x11 //Air Conditioner on/off
#define office_AC_temp		0x12 //Air Conditioner temperature
#define office_light		0x13 //ceiling light | N1DC130001
#define office_switchLed	0x14 //wall switch led
#define office_switch  		0x15 //wall switch
#define office_ir			0x16 //ir 
#define external_light		0x17 //light sensor [0-1024]
#define external_temp		0x1A //temperature [celsius /100] | N1DA1A0000
#define external_humidity	0x1B //humidity [%] | N1DA1B0000
#define external_pressure	0x1C //pressure [hPa /FIX THIS ((x-900) * 100?)] | N1DA1C0000
#define external_weather	0x1F //all devices | N1DA1F0099

// Bedroom
#define bedroom_node		0x2	 //N2DC02F0F0
#define bedroom_AC			0x23 //Air Conditioner on/off
#define bedroom_AC_temp		0x24 //Air Conditioner temperature
#define bedroom_light		0x25 //Ceiling light N2DC250001
#define bedroom_switchLed	0x26 //Wall switch led
#define bedroom_switch		0x27 //Wall switch
#define bedroom_ir			0x28 //ir

// Living
#define living_node			0x3	 //N3DC04F0F0\n
#define living_switch		0x31
#define living_switchLed	0x32 //N3DC320099\n 
#define living_mainLight	0x33 //N3DC330002\n - center
#define living_dicroLight	0x34 //N3DC340000\n - 2x4 dicro array
#define living_spotLight	0x35 //N3DC350000\n - spotlights
#define living_auxLight		0x36 //N3DC360000\n - guidance leds
#define living_fx			0x37 //N3DC370000\n - raw fx controller
#define living_AC			0x38 //N3DC380001\n
#define living_AC_temp		0x39 //N3DC390018\n

// Frontdesk
#define frontdesk_node		0x5