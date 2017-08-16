#include "iPV1000.h"

extern byte *z80_memory;
extern int ipv_vram;
extern int ipv_cgen;
extern int ipv_patn;
extern int ipv_fpat;
extern int psg_period[3];
extern SDL_Joystick *joy;

int ipv_jmode = 0;
int ipv_jstatus = 1;

int ipv_wait = 0;

byte io80_readm(int param, ushort address)
{
	ipv_wait++;
	return z80_memory[address];
}

void io80_writem(int param, ushort address, byte data)
{
	ipv_wait++;
	z80_memory[address] = data;
}

byte io80_readp(int param, ushort address)
{
	int ret = 0;

	int kmode[8];

	switch(address & 0xff) {
	case 0xfc:
		ipv_wait += 4;
		ret = ipv_jstatus;
		ipv_jstatus &= ~1;
		break;
	case 0xfd:
		{
			ipv_wait += 32;
			memset(kmode,0,sizeof(kmode));

			if(SDL_JoystickGetButton(joy,1)) {
				kmode[PV_TRIG1] = 1;	// TRIG1
			}
			if(SDL_JoystickGetButton(joy,2)) {
				kmode[PV_TRIG2] = 1;	// TRIG2
			}
			if(SDL_JoystickGetButton(joy,10)) {
				kmode[PV_SELECT] = 1;	// select
			}
			if(SDL_JoystickGetButton(joy,11)) {
				kmode[PV_START] = 1;	// start
			}

			if(SDL_JoystickGetButton(joy,8)) {
				kmode[PV_UP] = 1;		// up
			}
			if(SDL_JoystickGetButton(joy,6)) {
				kmode[PV_DOWN] = 1;		// down
			}
			if(SDL_JoystickGetButton(joy,7)) {
				kmode[PV_LEFT] = 1;		// left
			}
			if(SDL_JoystickGetButton(joy,9)) {
				kmode[PV_RIGHT] = 1;	// right
			}
		}
		if(ipv_jmode & 8) {
			ret |= kmode[PV_TRIG1];
			ret |= kmode[PV_TRIG2] << 1;
		}
		if(ipv_jmode & 4) {
			ret |= kmode[PV_LEFT];
			ret |= kmode[PV_UP] << 1;
		}
		if(ipv_jmode & 2) {
			ret |= kmode[PV_DOWN];
			ret |= kmode[PV_RIGHT] << 1;
		}
		if(ipv_jmode & 1) {
			ret |= kmode[PV_SELECT];
			ret |= kmode[PV_START] << 1;
		}
		break;
	/*default:
		printf("Unknown Port Read %02x\n",address);
		break;*/
	}

	return ret;
}

void io80_writep(int param, ushort address, byte data)
{
	switch(address & 0xff) {
	case 0xf8: case 0xf9: case 0xfa:
		ipv_wait += 16;
		psg_period[address & 3] = 0x3f - (data & 0x3f);
		break;
	case 0xfe:
		ipv_wait += 8;
		ipv_vram = data;
		ipv_cgen = data + (0x400 >> 8);
		break;
	case 0xff:
		ipv_wait += 8;
		ipv_patn = (data & 0x20);
		ipv_fpat = ((data & 0x10) != 0);
		break;
	case 0xfd:
		ipv_wait += 2;
		ipv_jmode = data;
		ipv_jstatus |= 2;
		break;
	/*default:
		printf("Unknown Port Write %02x data %02x\n",address,data);
		break;*/
	}
}