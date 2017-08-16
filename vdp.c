#include "iPV1000.h"

extern byte *z80_memory;
extern int ipv_vram;
extern int ipv_cgen;
extern int ipv_patn;
extern int ipv_fpat;

int rgb_table[] = { 0x00, 0xff };

void ipv_grefresh(SDL_Surface *sdl_screen)
{
	int x,y;

	int e_vr = ipv_vram << 8;

	for(y = 0; y < 24; y++) {
		for(x = 2; x < 30; x++) {
			int c = z80_memory[e_vr + (y * 32 + x)];
			int e_cg;
			if(ipv_fpat || c < 0xe0) {
				e_cg = ipv_patn << 8;
			} else {
				e_cg = ipv_cgen << 8;
				c &= 0x1f;
			}

			int vx = x << 3;
			int vy = y << 3;

			int i;

			for(i = 0; i < 8; i++) {
				int r,g,b;

				r = z80_memory[e_cg + c*32 + i +  8];
				g = z80_memory[e_cg + c*32 + i + 16];
				b = z80_memory[e_cg + c*32 + i + 24];

				int j;
				for(j = 0; j < 8; j++) {
					int p = 7-j;

					int rd = (r >> p) & 1;
					int gd = (g >> p) & 1;
					int bd = (b >> p) & 1;

					((Uint32 *)sdl_screen->pixels)[(vy + i) * (sdl_screen->pitch >> 2) + (vx + j)] = (rgb_table[bd] << 16) | (rgb_table[gd] << 8) | rgb_table[rd];
				}
			}
		}
	}
}