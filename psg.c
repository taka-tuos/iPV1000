#include "ipv1000.h"
#include "SDL.h"

SDL_AudioSpec Desired;
SDL_AudioSpec Obtained;

int psg_count[3];
int psg_period[3];
char psg_signal[3];

#define PSG_VOL 8192

#define FREQ	22050

int diff = (int)(1.3 * (double)(3.579*1000.0*1000.0) / (double)FREQ + 0.5);

void callback_psg(void *unused, Uint8 *stream, int len)
{
	int i;
	int chs;
	static unsigned int step= 0;
	Sint16 *frames = (Sint16 *) stream;
	int framesize = len / 2;
	for (i = 0; i < framesize; i++, step++) {
		int vol = 0;
		for(int j = 0; j < 3; j++) {
			if(!psg_period[j]) {
				continue;
			}
			char prev_signal = psg_signal[j];
			int prev_count = psg_count[j];
			psg_count[j] -= diff;
			if(psg_count[j] < 0) {
				psg_count[j] += psg_period[j] << 8;
				psg_signal[j] = !psg_signal[j];
			}
			int vol_tmp = (prev_signal != psg_signal[j] && prev_count < diff) ? (PSG_VOL * (2 * prev_count - diff)) / diff : PSG_VOL;
			vol += prev_signal ? vol_tmp : -vol_tmp;
		}
		*frames++ += vol;
	}
}

void reset_psg(void)
{
	int i;

	for(i = 0; i < 3; i++) {
		psg_count[i] = 0;
		psg_period[i] = 0;
		psg_signal[i] = 0;
	}
}

void init_psg(void)
{
	reset_psg();

	Desired.freq= FREQ; /* Sampling rate: 22050Hz */
	Desired.format= AUDIO_S16; /* 16-bit signed audio */
	Desired.channels= 1; /* Mono */
	Desired.samples= FREQ/100; /* Buffer size: 2205000B = 10 sec. */
	Desired.callback = callback_psg;
	Desired.userdata = NULL;

	SDL_OpenAudio(&Desired, &Obtained);
	SDL_PauseAudio(0);
}