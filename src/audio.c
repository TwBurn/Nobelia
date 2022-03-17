#include <strings.h>
#include <csd.h>
#include <sysio.h>
#include <cdfm.h>
#include <stdio.h>
#include <memory.h>
#include "audio.h"

#define SIG_AUDIO_SM  0x0200
#define SIG_AUDIO_PCB 0x0400
#define SIG_AUDIO_PCL 0x0800

#define MUSIC_CHUNKS 18
#define MUSIC_SIZE   2304
#define MUSIC_WORDS (MUSIC_SIZE / 4)
#define MUSIC_PCL_COUNT 4

#define AUDIO_HEADER_L 0xC000C000
#define AUDIO_HEADER_R 0x00C000C0

/* Play Data */
int audioPath;
SoundMap smPlay;
STAT_BLK smStat;
SoundmapDesc *smInfo;
u_short lastLoop;

/* Music Data */
int musicRtf;
char *musicSilence;
char *musicBuffer;
PCB musicPcb;
PCL *musicCil[16];
PCL  musicPcl[MUSIC_PCL_COUNT];
int curChannel = -1;
int pclPlay, pclRead;
int musicStatus = MUSIC_STOP;

/* SFX Data */
char* sfxBuffer;
SoundEffect sfxActive[SFX_LIMIT];
int sfxCount = 0;


void playSfx(start, end)
	int start, end;
{
	int i = 0; int j;

	if (sfxCount && end <= SFX_IGNORE) {
		return;
	}
	else if (sfxCount && sfxActive[0].last <= SFX_IGNORE) {
		sfxActive[0].current = start;
		sfxActive[0].last = end;
		return;
	}

	while(i < sfxCount) {
		if      (sfxActive[i].last == end) return;
		else if (sfxActive[i].last <  end) break;
		i++;
	}
	if (i >= SFX_LIMIT) return;
	
	/* Insert at i */
	for (j = (SFX_LIMIT - 1); j > i; j--) {
		sfxActive[j] = sfxActive[j-1];
	}
	sfxActive[i].current = start;
	sfxActive[i].last    = end;

	if (sfxCount < SFX_LIMIT) sfxCount++;
}

void stepSfx()
{
	int i = 0; int j;
	while (i < sfxCount) {
		if (sfxActive[i].current == sfxActive[i].last) {
			for (j = i; j < (SFX_LIMIT - 1); j++) {
				sfxActive[j] = sfxActive[j + 1];
			}
			sfxActive[SFX_LIMIT - 1].current = 0; sfxActive[SFX_LIMIT - 1].last = 0;
			sfxCount--;
		}
		else {
			sfxActive[i].current++;
			i++;
		}
	}
}

void startAudio() {
	memcpy(smPlay.buffer, sfxBuffer, MUSIC_SIZE);
	sd_loop(audioPath, smPlay.id, 0, 1, 0x0FFF);
	sm_out(audioPath, smPlay.id, &smStat);
	smInfo = sm_info(audioPath, smPlay.id);
	lastLoop = 0;
}

void createSilence(buffer, header)
	register u_int *buffer;
	register u_int header;
{
	int i;
	for (i = 0; i < MUSIC_CHUNKS; i++)
	{
		/* HEADER */
		*buffer++ = header; *buffer++ = header; *buffer++ = header; *buffer++ = header;

		/* DATA */
		*buffer++ = 0; *buffer++ = 0; *buffer++ = 0; *buffer++ = 0;
		*buffer++ = 0; *buffer++ = 0; *buffer++ = 0; *buffer++ = 0;
		*buffer++ = 0; *buffer++ = 0; *buffer++ = 0; *buffer++ = 0;
		*buffer++ = 0; *buffer++ = 0; *buffer++ = 0; *buffer++ = 0;
		*buffer++ = 0; *buffer++ = 0; *buffer++ = 0; *buffer++ = 0;
		*buffer++ = 0; *buffer++ = 0; *buffer++ = 0; *buffer++ = 0;
		*buffer++ = 0; *buffer++ = 0; *buffer++ = 0; *buffer++ = 0;
	}
}

void mixSfxMusic() {
	register u_int *dst, *sfx, *mus;
	register int i;

	dst = (u_int*)(smPlay.buffer);
	
	if (pclPlay == pclRead) {
		mus = (u_int*)(musicSilence);
		if (musicStatus == MUSIC_STOP) { }
		else if (musicStatus) {
			musicStatus--;
		}
		else if (curChannel >= 0) {
			playMusic(curChannel);
		}
	}
	else {
		mus = (u_int*)(musicBuffer + MUSIC_SIZE * pclPlay);
		musicPcl[pclPlay].PCL_Ctrl = 0;
		musicPcl[pclPlay].PCL_Cnt = 0;
		pclPlay = (pclPlay + 1) & 0x03; /* MOD MUSIC_PCL_COUNT */
	}

	for (i = 0; i < MUSIC_CHUNKS; i++)
	{
		if (sfxCount == 0) {
			/* No SFX */
			sfx = (u_int*)(sfxBuffer + (i * 128));
		}
		else {
			/* Mix multiple SFX */
			sfx = (u_int*)(sfxBuffer + (i * 128) + (MUSIC_SIZE * sfxActive[i % sfxCount].current));
		}
		/* MIXING IF DATA IS MASKED ALREADY */
		*dst++ = *sfx++ | *mus++; *dst++ = *sfx++ | *mus++; *dst++ = *sfx++ | *mus++; *dst++ = *sfx++ | *mus++; *dst++ = *sfx++ | *mus++; *dst++ = *sfx++ | *mus++; *dst++ = *sfx++ | *mus++; *dst++ = *sfx++ | *mus++;
		*dst++ = *sfx++ | *mus++; *dst++ = *sfx++ | *mus++; *dst++ = *sfx++ | *mus++; *dst++ = *sfx++ | *mus++; *dst++ = *sfx++ | *mus++; *dst++ = *sfx++ | *mus++; *dst++ = *sfx++ | *mus++; *dst++ = *sfx++ | *mus++;
		*dst++ = *sfx++ | *mus++; *dst++ = *sfx++ | *mus++; *dst++ = *sfx++ | *mus++; *dst++ = *sfx++ | *mus++; *dst++ = *sfx++ | *mus++; *dst++ = *sfx++ | *mus++; *dst++ = *sfx++ | *mus++; *dst++ = *sfx++ | *mus++;
		*dst++ = *sfx++ | *mus++; *dst++ = *sfx++ | *mus++; *dst++ = *sfx++ | *mus++; *dst++ = *sfx++ | *mus++; *dst++ = *sfx++ | *mus++; *dst++ = *sfx++ | *mus++; *dst++ = *sfx++ | *mus++; *dst++ = *sfx++ | *mus++;

		/* MIXING WITH MASKING */
		/* HEADER */
		/*
		*dst++ = (*sfx++ & 0xFF00FF00) | (*mus++ & 0x00FF00FF); *dst++ = (*sfx++ & 0xFF00FF00) | (*mus++ & 0x00FF00FF); *dst++ = (*sfx++ & 0xFF00FF00) | (*mus++ & 0x00FF00FF); *dst++ = (*sfx++ & 0xFF00FF00) | (*mus++ & 0x00FF00FF);
		*/
		/* DATA *//*
		/*dst++ = (*sfx++ & 0x0F0F0F0F) | (*mus++ & 0xF0F0F0F0); *dst++ = (*sfx++ & 0x0F0F0F0F) | (*mus++ & 0xF0F0F0F0); *dst++ = (*sfx++ & 0x0F0F0F0F) | (*mus++ & 0xF0F0F0F0); *dst++ = (*sfx++ & 0x0F0F0F0F) | (*mus++ & 0xF0F0F0F0);
		*dst++ = (*sfx++ & 0x0F0F0F0F) | (*mus++ & 0xF0F0F0F0); *dst++ = (*sfx++ & 0x0F0F0F0F) | (*mus++ & 0xF0F0F0F0); *dst++ = (*sfx++ & 0x0F0F0F0F) | (*mus++ & 0xF0F0F0F0); *dst++ = (*sfx++ & 0x0F0F0F0F) | (*mus++ & 0xF0F0F0F0);
		*dst++ = (*sfx++ & 0x0F0F0F0F) | (*mus++ & 0xF0F0F0F0); *dst++ = (*sfx++ & 0x0F0F0F0F) | (*mus++ & 0xF0F0F0F0); *dst++ = (*sfx++ & 0x0F0F0F0F) | (*mus++ & 0xF0F0F0F0); *dst++ = (*sfx++ & 0x0F0F0F0F) | (*mus++ & 0xF0F0F0F0);
		*dst++ = (*sfx++ & 0x0F0F0F0F) | (*mus++ & 0xF0F0F0F0); *dst++ = (*sfx++ & 0x0F0F0F0F) | (*mus++ & 0xF0F0F0F0); *dst++ = (*sfx++ & 0x0F0F0F0F) | (*mus++ & 0xF0F0F0F0); *dst++ = (*sfx++ & 0x0F0F0F0F) | (*mus++ & 0xF0F0F0F0);
		*dst++ = (*sfx++ & 0x0F0F0F0F) | (*mus++ & 0xF0F0F0F0); *dst++ = (*sfx++ & 0x0F0F0F0F) | (*mus++ & 0xF0F0F0F0); *dst++ = (*sfx++ & 0x0F0F0F0F) | (*mus++ & 0xF0F0F0F0); *dst++ = (*sfx++ & 0x0F0F0F0F) | (*mus++ & 0xF0F0F0F0);
		*dst++ = (*sfx++ & 0x0F0F0F0F) | (*mus++ & 0xF0F0F0F0); *dst++ = (*sfx++ & 0x0F0F0F0F) | (*mus++ & 0xF0F0F0F0); *dst++ = (*sfx++ & 0x0F0F0F0F) | (*mus++ & 0xF0F0F0F0); *dst++ = (*sfx++ & 0x0F0F0F0F) | (*mus++ & 0xF0F0F0F0);
		*dst++ = (*sfx++ & 0x0F0F0F0F) | (*mus++ & 0xF0F0F0F0); *dst++ = (*sfx++ & 0x0F0F0F0F) | (*mus++ & 0xF0F0F0F0); *dst++ = (*sfx++ & 0x0F0F0F0F) | (*mus++ & 0xF0F0F0F0); *dst++ = (*sfx++ & 0x0F0F0F0F) | (*mus++ & 0xF0F0F0F0);
		*/
	}

	stepSfx();
}

void stepAudio() {
	if (lastLoop == smInfo->smd_lcntr) return;

	mixSfxMusic();

	if (!(smInfo->smd_lcntr & 0x0800)) {
		smInfo->smd_lcntr = 0x0FFF;
	}
	lastLoop = smInfo->smd_lcntr;
}
/*
void copySfx() {
	register u_int* src = (u_int*)(sfxBuffer + MUSIC_SIZE * sfxCur);
	register u_int* dst = (u_int*)(smPlay.buffer);

	register int i;
	/* Copy 576 groups of 4 bytes - in 18 chunks of 32 writes each *//*
	for (i = 0; i < MUSIC_CHUNKS; i++)
	{
		*dst++=*src++; *dst++=*src++; *dst++=*src++; *dst++=*src++; *dst++=*src++; *dst++=*src++; *dst++=*src++; *dst++=*src++;
		*dst++=*src++; *dst++=*src++; *dst++=*src++; *dst++=*src++; *dst++=*src++; *dst++=*src++; *dst++=*src++; *dst++=*src++;
		*dst++=*src++; *dst++=*src++; *dst++=*src++; *dst++=*src++; *dst++=*src++; *dst++=*src++; *dst++=*src++; *dst++=*src++;
		*dst++=*src++; *dst++=*src++; *dst++=*src++; *dst++=*src++; *dst++=*src++; *dst++=*src++; *dst++=*src++; *dst++=*src++;
	}
	

	if (sfxCur < sfxEnd) {
		sfxCur++;
	}
	else {
		sfxCur = 0;	sfxEnd = 0;
	}
}

void mixMusic() {
	register u_int* src = (u_int*)(musicBuffer + MUSIC_SIZE * pclPlay);
	register u_int* dst = (u_int*)(smPlay.buffer);
	register int i;

	if (pclPlay == pclRead) return;

	for (i = 0; i < MUSIC_CHUNKS; i++)
	{
		/* HEADER */
		/**dst++|=*src++; *dst++|=*src++; *dst++|=*src++; *dst++|=*src++;*//*
		*dst++ = (*dst & 0xFF00FF00) | (*src++ & 0x00FF00FF); *dst++ = (*dst & 0xFF00FF00) | (*src++ & 0x00FF00FF); *dst++ = (*dst & 0xFF00FF00) | (*src++ & 0x00FF00FF); *dst++ = (*dst & 0xFF00FF00) | (*src++ & 0x00FF00FF);
		
		/* DATA *//*
		*dst++ = (*dst & 0x0F0F0F0F) | (*src++ & 0xF0F0F0F0); *dst++ = (*dst & 0x0F0F0F0F) | (*src++ & 0xF0F0F0F0); *dst++ = (*dst & 0x0F0F0F0F) | (*src++ & 0xF0F0F0F0); *dst++ = (*dst & 0x0F0F0F0F) | (*src++ & 0xF0F0F0F0);
		*dst++ = (*dst & 0x0F0F0F0F) | (*src++ & 0xF0F0F0F0); *dst++ = (*dst & 0x0F0F0F0F) | (*src++ & 0xF0F0F0F0); *dst++ = (*dst & 0x0F0F0F0F) | (*src++ & 0xF0F0F0F0); *dst++ = (*dst & 0x0F0F0F0F) | (*src++ & 0xF0F0F0F0);
		*dst++ = (*dst & 0x0F0F0F0F) | (*src++ & 0xF0F0F0F0); *dst++ = (*dst & 0x0F0F0F0F) | (*src++ & 0xF0F0F0F0); *dst++ = (*dst & 0x0F0F0F0F) | (*src++ & 0xF0F0F0F0); *dst++ = (*dst & 0x0F0F0F0F) | (*src++ & 0xF0F0F0F0);
		*dst++ = (*dst & 0x0F0F0F0F) | (*src++ & 0xF0F0F0F0); *dst++ = (*dst & 0x0F0F0F0F) | (*src++ & 0xF0F0F0F0); *dst++ = (*dst & 0x0F0F0F0F) | (*src++ & 0xF0F0F0F0); *dst++ = (*dst & 0x0F0F0F0F) | (*src++ & 0xF0F0F0F0);
		*dst++ = (*dst & 0x0F0F0F0F) | (*src++ & 0xF0F0F0F0); *dst++ = (*dst & 0x0F0F0F0F) | (*src++ & 0xF0F0F0F0); *dst++ = (*dst & 0x0F0F0F0F) | (*src++ & 0xF0F0F0F0); *dst++ = (*dst & 0x0F0F0F0F) | (*src++ & 0xF0F0F0F0);
		*dst++ = (*dst & 0x0F0F0F0F) | (*src++ & 0xF0F0F0F0); *dst++ = (*dst & 0x0F0F0F0F) | (*src++ & 0xF0F0F0F0); *dst++ = (*dst & 0x0F0F0F0F) | (*src++ & 0xF0F0F0F0); *dst++ = (*dst & 0x0F0F0F0F) | (*src++ & 0xF0F0F0F0);
		*dst++ = (*dst & 0x0F0F0F0F) | (*src++ & 0xF0F0F0F0); *dst++ = (*dst & 0x0F0F0F0F) | (*src++ & 0xF0F0F0F0); *dst++ = (*dst & 0x0F0F0F0F) | (*src++ & 0xF0F0F0F0); *dst++ = (*dst & 0x0F0F0F0F) | (*src++ & 0xF0F0F0F0);
		/*dst++|=*src++; *dst++|=*src++; *dst++|=*src++; *dst++|=*src++;
		*dst++|=*src++; *dst++|=*src++; *dst++|=*src++; *dst++|=*src++; *dst++|=*src++; *dst++|=*src++; *dst++|=*src++; *dst++|=*src++;
		*dst++|=*src++; *dst++|=*src++; *dst++|=*src++; *dst++|=*src++; *dst++|=*src++; *dst++|=*src++; *dst++|=*src++; *dst++|=*src++;
		*dst++|=*src++; *dst++|=*src++; *dst++|=*src++; *dst++|=*src++; *dst++|=*src++; *dst++|=*src++; *dst++|=*src++; *dst++|=*src++;*//*
	}

	/* Update Play information *//*
	musicPcl[pclPlay].PCL_Ctrl = 0;
	musicPcl[pclPlay].PCL_Cnt = 0;
	pclPlay = (pclPlay + 1) & 0x03; /* MOD MUSIC_PCL_COUNT *//*
}
*/

void initPlay()
{
	smPlay.id = sm_creat(audioPath, D_CSTRO, MUSIC_CHUNKS, &smPlay.buffer);
	smStat.asy_sig = 0; /*SIG_AUDIO_SM;*/
}

void initMusic()
{
	int i;
	musicSilence = (char*)srqcmem(MUSIC_SIZE * (MUSIC_PCL_COUNT + 1), VIDEO1);
	musicBuffer  = musicSilence + MUSIC_SIZE;

	createSilence(musicSilence, AUDIO_HEADER_R);

	/* Setup PCB */
	musicPcb.PCB_Rec   = 1;
	musicPcb.PCB_Sig   = SIG_AUDIO_PCB;
	
	musicPcb.PCB_Video = 0;
	musicPcb.PCB_Audio = 0;
	musicPcb.PCB_Data  = 0;

	/*Set up PCL */
	for (i = 0; i < MUSIC_PCL_COUNT; i++) {
		musicPcl[i].PCL_Ctrl = 0;
		musicPcl[i].PCL_Buf = musicBuffer + i * MUSIC_SIZE;
		musicPcl[i].PCL_BufSz = 1;
		musicPcl[i].PCL_Sig = SIG_AUDIO_PCL + i;
		musicPcl[i].PCL_Nxt = &musicPcl[(i + 1) & 0x03]; /* MOD MUSIC_PCL_COUNT */
		musicPcl[i].PCL_Err = 0;
		musicPcl[i].PCL_Cnt = 0;
	}
	
	/* Set all channels to use the same PCL for streaming */
	for (i = 0; i < 16; i++) musicCil[i] = &musicPcl[0];
}

void initSfx()
{
	int i;
	for (i = 0; i < SFX_LIMIT; i++) {
		sfxActive[i].current = 0;
		sfxActive[i].last = 0;
	}
	
	sfxBuffer = (char*)srqcmem(SFX_SIZE, VIDEO2);
	createSilence(sfxBuffer, AUDIO_HEADER_L);
}

int readSfx(file)
	int file;
{
	int result = read(file, sfxBuffer, SFX_SIZE);
	createSilence(sfxBuffer, AUDIO_HEADER_L);
	return result;
}

void stopMusic() {
	if (!musicRtf) return;
	
	close(musicRtf);
	
	musicRtf = 0;
	curChannel = -1;
	pclPlay = 0; pclRead = 0;
	musicStatus = MUSIC_STOP;
	mixSfxMusic(); /* Silences audio buffer */
}

void playMusic(channel)
	int channel;
{
	int i;

	if (musicStatus > 0 && curChannel == channel) return;

	stopMusic();

	musicRtf = open("MUSIC.RTF", READ_);
	musicPcb.PCB_Chan  = 1 << channel;
	musicPcb.PCB_AChan = 0; /* Set to 0 to disable audio output */
	musicPcb.PCB_Audio = musicCil;
	musicPcb.PCB_Stat = 0;

	for (i = 0; i < MUSIC_PCL_COUNT; i++) {
		musicPcl[i].PCL_Ctrl = 0;
		musicPcl[i].PCL_Cnt  = 0;
	}

	curChannel = channel;
	ss_play(musicRtf, &musicPcb);
}


void initAudio()
{
	char *devName = csd_devname(DT_AUDIO, 1); /* Get Audio Device Name */
	audioPath = open(devName, UPDAT_);        /* Open Audio Device */
	free(devName);                            /* Release memory */
	/* NOTE SC_ATTEN( R->L, R->R, L->R, L->L) */
	sc_atten(audioPath, 0x00000000); /* Full Mix */ 
	/*sc_atten(audioPath, 0x40404040); /* Half Mix */ 
	/*sc_atten(audioPath, 0x20206060); /* Mix Musix less loud, SFX more loud */ 
	/*sc_atten(audioPath, 0x00800080); /* Normal Stereo */
	initPlay();
	initMusic();
	initSfx();
}

void closeAudio()
{
	stopMusic();
	sm_off(audioPath);
	sm_close(audioPath, smPlay.id);
	close(audioPath);  /* Close Audio Device */
}

void setAttenuation(music, sfx)
	u_char music, sfx;
{
	u_int att = (sfx << 24) | (sfx << 16) | (music << 8) | music;
	sc_atten(audioPath, att);
}

void handleAudioSignal(sigCode)
	u_short sigCode;
{
	u_short sigNum;
	if (sigCode & SIG_AUDIO_PCL) {
		sigNum = sigCode & 0x00FF;

		musicPcl[sigNum].PCL_Ctrl = 0;
		musicPcl[sigNum].PCL_Cnt  = 0;

		pclRead = (sigCode + 1) & 0x03; /* MOD MUSIC_PCL_COUNT */
		musicStatus = MUSIC_PLAY;
	}
	stepAudio();
}
