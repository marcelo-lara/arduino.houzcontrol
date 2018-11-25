/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Sony Remote Control Keys

// DVR keys
#define irDvr1 0xB59		//dvr: keypad 1
#define irDvr2 0x80B59		//dvr: keypad 2
#define irDvr3 0x40B59		//dvr: keypad 3
#define irDvr4 0xC0B59		//dvr: keypad 4
#define irDvr5 0x20B59		//dvr: keypad 5
#define irDvr6 0xA0B59		//dvr: keypad 6
#define irDvr7 0x60B59		//dvr: keypad 7
#define irDvr8 0xE0B59		//dvr: keypad 8
#define irDvr9 0x10B59		//dvr: keypad 9
#define irDvr0 0xD0B59		//dvr: keypad 0
#define irDvrEnter 0x90B59	//dvr: keypad Enter
#define irDvrA 0xDAB59		//dvr: key A
#define irDvrB 0x4EB59		//dvr: key B

#define irDvrUp 0xFAB59		//dvr: joystick up
#define irDvrRight 0x86B59	//dvr: joystick right
#define irDvrLeft 0x46B59	//dvr: joystick left
#define irDvrDown 0x6B59	//dvr: joystick down
#define irDvrCenter 0x7AB59	//dvr: joystick enter


class HouzInfrared{
	public:
		HouzInfrared(byte IR_LED_pin);
		void send(u32 device);
	private:
		byte IR_LED_PIN;
};