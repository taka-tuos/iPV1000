#include "iPV1000.h"
#include "libgpu.h"

#define FPS				60.0f
#define MHZ				3.579f

int ipv_vram = 0xb8;
int ipv_cgen = 0xbc;
int ipv_patn = 0;
int ipv_fpat = 0;

extern int ipv_jstatus;

byte *z80_memory;

int poll_event(SDL_Event *sdl_event)
{
	if(SDL_PollEvent(sdl_event)) {
		switch (sdl_event->type) {
		case SDL_QUIT:
			return 1;
		}
	}

	return 0;
}

void adjustFPS(void) {
	static unsigned long maetime=0;
	static int frame=0;
	long sleeptime;
	if(!maetime) SDL_GetTicks();
	frame++;
	sleeptime=(frame<FPS)?
		(maetime+(long)((float)frame*(1000.0f/FPS))-SDL_GetTicks()):
		(maetime+1000-SDL_GetTicks());
	if(sleeptime>0)SDL_Delay(sleeptime);
	if(frame>=FPS) {
		frame=0;
		maetime=SDL_GetTicks();
	}
}

extern int ipv_wait;

void z80_execute(Z80Context *z80)
{
	int elim;
	unsigned int tstates;
	
	int intf = 0;
	
	tstates = z80->tstates;
	
	for(elim = 0; (int)((unsigned long)z80->tstates - tstates) < MHZ*1000.0f*1000.0f/FPS;elim++) {
		Z80Execute(z80);
		
		z80->tstates += ipv_wait;
		ipv_wait = 0;
		
		int lm = (MHZ*1000.0f*1000.0f/FPS) - (int)((unsigned long)z80->tstates - tstates);

		if(lm < 12800 && abs(lm % 800) <= 24 && !intf) {
			Z80INT(z80,0);
			intf = 1;
		} else {
			intf = 0;
		}
	}
	ipv_jstatus |= 1;
}

struct dirent *roms[512];

SDL_Joystick *joy;

void chld_rom(SDL_Surface *sdl_screen)
{
	SDL_Event sdl_event;

	DIR *dir = opendir("./roms/");
	
	int i;
	for(i = 0; i < 512; i++) {
		roms[i] = 0;
	}

	readdir(dir);
	readdir(dir);

	for(i = 0; i < 512; i++) {
		struct dirent *p = readdir(dir);
		if(!p) break;
		roms[i] = (struct dirent *)malloc(sizeof(struct dirent));
		memcpy(roms[i],p,sizeof(struct dirent));
	}
	
	closedir(dir);
	
	int j = 0;

	int k1=0,k2=0;
	int l = 0;

	while(1) {
		memset(sdl_screen->pixels,0,sdl_screen->pitch*sdl_screen->h);

		if(poll_event(&sdl_event)) {
			SDL_Quit();
			exit(0);
		}

		libgpu_puts(sdl_screen,0,0,0xffffff,"Please select the ROM.");

		for(i = l; i < 512; i++) {
			if(!roms[i]) break;

			libgpu_puts(sdl_screen,12,(i-l)*8+16,0xffffff,roms[i]->d_name);
		}

		libgpu_puts(sdl_screen,0,j*8+16,0xffffff,">");

		if(SDL_JoystickGetButton(joy,8)) {
			if(k1 == 0) j--;
			k1 = 1;
		} else {
			k1 = 0;
		}
		if(SDL_JoystickGetButton(joy,6)) {
			if(k2 == 0) j++;
			k2 = 1;
		} else {
			k2 = 0;
		}
		if(SDL_JoystickGetButton(joy,1)) break;

		if(j < 0 && l > 0) l--;
		if(j > 21 && l < 512) l++;

		if(j < 0) j = 0;
		if(j > 21) j = 21;
		if(j >= i-l) j = i-l-1;

		SDL_UpdateRect(sdl_screen,0,0,0,0);
		
		SDL_Delay(20);
	}
	
	char romname[256];
	sprintf(romname,"roms/%s",roms[j+l]->d_name);

	FILE *fp = fopen(romname,"rb");
	fread(z80_memory,1,0x8000,fp);
	fclose(fp);
	
	memset(sdl_screen->pixels,0,sdl_screen->pitch*sdl_screen->h);
}

void z80_vmreset(Z80Context *z80)
{
	memset(z80,0,sizeof(Z80Context));
	
	Z80RESET(z80);

	z80->PC = 0x0000;
	z80->I = 0;
	z80->R = 0;
	z80->R1.wr.IX = 0xffff;
	z80->R1.wr.IY = 0xffff;
	z80->R1.br.F = 0x40;
	z80->memRead = io80_readm;
	z80->memWrite = io80_writem;
	z80->ioRead = io80_readp;
	z80->ioWrite = io80_writep;

	ipv_fpat = 0;
	ipv_patn = 0;
	ipv_cgen = 0xbc;
	ipv_vram = 0xb8;
}

int main(int argc, char *argv[])
{
	SDL_Surface *sdl_screen;
	SDL_Event sdl_event;

	SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO|SDL_INIT_JOYSTICK);
	
	joy = SDL_JoystickOpen(0);

	sdl_screen = SDL_SetVideoMode(256,192,32,SDL_SWSURFACE);

	init_psg();

	z80_memory = (byte *)malloc(0x10000);
	memset(z80_memory,0xff,0x10000);

	Z80Context z80;
	
	memset(&z80,0,sizeof(Z80Context));
	
	Z80RESET(&z80);

	z80.PC = 0x0000;
	z80.I = 0;
	z80.R = 0;
	z80.R1.wr.IX = 0xffff;
	z80.R1.wr.IY = 0xffff;
	z80.R1.br.F = 0x40;
	z80.memRead = io80_readm;
	z80.memWrite = io80_writem;
	z80.ioRead = io80_readp;
	z80.ioWrite = io80_writep;

	libgpu_init();
	
	memset(z80_memory,0xff,0x8000);
	
	chld_rom(sdl_screen);
	
	int s_frame = 0;

	while(!poll_event(&sdl_event)) {
		if(SDL_JoystickGetButton(joy,4) && SDL_JoystickGetButton(joy,5)) {
			reset_psg();
			
			memset(z80_memory,0xff,0x10000);
			
			chld_rom(sdl_screen);
			
			z80_vmreset(&z80);
			
			memset(sdl_screen->pixels,0,sdl_screen->pitch*sdl_screen->h);
			
			libgpu_puts(sdl_screen,0,192-8,0xffffff,"Please wait...");
			
			SDL_UpdateRect(sdl_screen,0,0,0,0);
			
			memset(sdl_screen->pixels,0,sdl_screen->pitch*sdl_screen->h);
			
			memset(z80_memory+(ipv_vram<<8),0,32*24);
		}
		
		if(SDL_JoystickGetButton(joy,0) && SDL_JoystickGetButton(joy,3)) {
			reset_psg();
			
			z80_vmreset(&z80);
			
			memset(sdl_screen->pixels,0,sdl_screen->pitch*sdl_screen->h);
			
			libgpu_puts(sdl_screen,0,192-8,0xffffff,"Please wait...");
			
			SDL_UpdateRect(sdl_screen,0,0,0,0);
			
			memset(sdl_screen->pixels,0,sdl_screen->pitch*sdl_screen->h);
			
			memset(z80_memory+(ipv_vram<<8),0,32*24);
		}
		z80_execute(&z80);
		adjustFPS();
		if((s_frame & 1) == 0) ipv_grefresh(sdl_screen);
		SDL_UpdateRect(sdl_screen,0,0,0,0);
		s_frame++;
	}

	SDL_JoystickClose(joy);

	SDL_Quit();

	return 0;
}