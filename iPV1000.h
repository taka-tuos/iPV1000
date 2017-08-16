#include "SDL.h"
#include "z80.h"
#include <dirent.h>

byte io80_readm(int param, ushort address);
void io80_writem(int param, ushort address, byte data);
byte io80_readp(int param, ushort address);
void io80_writep(int param, ushort address, byte data);

void ipv_grefresh(SDL_Surface *sdl_screen);

void init_psg(void);
void reset_psg(void);

enum {
	PV_TRIG1,
	PV_TRIG2,
	PV_UP,
	PV_LEFT,
	PV_RIGHT,
	PV_DOWN,
	PV_START,
	PV_SELECT,
};
