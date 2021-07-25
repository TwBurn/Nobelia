#include <stdio.h>
#include "sys.h"
#include "audio.h"

#define SIG_AUDIO      0xA00
#define SIG_AUDIO_SM   0xA01
#define SIG_AUDIO_RTF  0xA02
#define SIG_AUDIO_SEEK 0xA03

#define SIG_AUDIO_PCL1 0xA11
#define SIG_AUDIO_PCL2 0xA12

#define MUSIC_BLOCKS 1
#define MUSIC_CHUNKS 18 * MUSIC_BLOCKS
#define MUSIC_SIZE   2304 * MUSIC_BLOCKS
#define MUSIC_WORDS  (MUSIC_SIZE / 4)

int audioPath;

/*Soundmaps; two to stream music into, two for mixing, one for the SFX buffer */
int   smMusic1_id,   smMusic2_id,   smMix1_id,   smMix2_id,   smSfx_id;
char *smMusic1_buf, *smMusic2_buf, *smMix1_buf, *smMix2_buf, *smSfx_buf;

int musicRtf;
PCB musicPcb;
STAT_BLK musicStat, musicSeek;

PCL *musicCil[16];

PCL musicPcl1, musicPcl2;

int sfxCur, sfxEnd;

void playSfx(start, size)
	int start, size;
{
	/* Play sound effect if it's priority is greater than the current one */
	
	if (start > sfxCur) {
		sfxCur = start;
		sfxEnd = (start + size) - 1;
	}
}

void initSfx()
{
	FILE *file;

	sfxCur = 0;
	sfxEnd = 0;
	
	smSfx_id = sm_creat(audioPath, D_CSTRO, 18 * SFX_ENTRIES, &smSfx_buf);	
}

void loadSfx(file)
	int file;
{
	read(file, smSfx_buf, SFX_SIZE);
}

void initMusic()
{
	int i;
/* Setup PCB */
	musicPcb.PCB_Rec   = 1;
	musicPcb.PCB_Sig   = SIG_AUDIO_RTF;
	
	musicPcb.PCB_Video = 0;
	musicPcb.PCB_Audio = 0;
	musicPcb.PCB_Data  = 0;

	musicStat.asy_sig = 0;/*SIG_AUDIO_SM;*/
	musicSeek.asy_sig = SIG_AUDIO_SEEK;

/*Set up Soundmap, PCL */
	smMusic1_id = sm_creat(audioPath, D_CSTRO, MUSIC_CHUNKS, &smMusic1_buf);
	smMusic2_id = sm_creat(audioPath, D_CSTRO, MUSIC_CHUNKS, &smMusic2_buf);

	smMix1_id = sm_creat(audioPath, D_CSTRO, MUSIC_CHUNKS, &smMix1_buf);
	smMix2_id = sm_creat(audioPath, D_CSTRO, MUSIC_CHUNKS, &smMix2_buf);

	musicPcl1.PCL_Ctrl = 0;
	musicPcl1.PCL_Buf = smMusic1_buf;
	musicPcl1.PCL_BufSz = MUSIC_BLOCKS;
	musicPcl1.PCL_Sig = SIG_AUDIO_PCL1;
	musicPcl1.PCL_Nxt = &musicPcl2;
	musicPcl1.PCL_Err = 0;
	musicPcl1.PCL_Cnt = 0;

	musicPcl2.PCL_Ctrl = 0;
	musicPcl2.PCL_Buf = smMusic2_buf;
	musicPcl2.PCL_BufSz = MUSIC_BLOCKS;
	musicPcl2.PCL_Sig = SIG_AUDIO_PCL2;
	musicPcl2.PCL_Nxt = &musicPcl1;
	musicPcl1.PCL_Err = 0;
	musicPcl1.PCL_Cnt = 0;

	/* Set all channels to use the same PCL for streaming */
	for (i = 0; i < 16; i++) musicCil[i] = &musicPcl1;
}

void playMusic(channel)
	int channel;
{
	musicRtf = open("MUSIC.RTF", READ_);
	musicPcb.PCB_Chan  = 1 << channel;
	musicPcb.PCB_AChan = 1 << channel;
	musicPcb.PCB_Audio = 0;
	musicPcb.PCB_Stat = 0;

	ss_play(musicRtf, &musicPcb);
}

void streamMusic(channel)
	int channel;
{
	musicRtf = open("MUSIC.RTF", READ_);
	musicPcb.PCB_Chan  = 1 << channel;
	musicPcb.PCB_AChan = 0; /* Set to 0 to disable audio output */
	musicPcb.PCB_Audio = musicCil;
	musicPcb.PCB_Stat = 0;

	musicPcl1.PCL_Ctrl = 0;
	musicPcl1.PCL_Cnt  = 0;
	musicPcl2.PCL_Ctrl = 0;
	musicPcl2.PCL_Cnt  = 0;
	
	ss_play(musicRtf, &musicPcb);
}

void stopMusic()
{
	/*ss_abort(musicRtf);*/
	close(musicRtf);
}

void mixSfx(smMusic_id, smMix_id)
	int smMusic_id, smMix_id;
{
	sd_smix(audioPath, smSfx_id, smMusic_id, smMix_id, sfxCur * MUSIC_BLOCKS, MIX_L1R2);

	if (sfxCur < sfxEnd) {
		sfxCur++;
	}
	else {
		sfxCur = 0;
		sfxEnd = 0;
	}
}

void mixAudio(musicBuffer, sfxBuffer)
u_int *musicBuffer, *sfxBuffer;
{
	int i = 0;
	while (i < MUSIC_WORDS) {
		if (i % 32 < 4) {
			/* Mix header */
			musicBuffer[i] = (musicBuffer[i] & 0xFF00FF00) | (sfxBuffer[i] & 0x00FF00FF);
		}
		else {
			/* Mix data */
			musicBuffer[i] |= sfxBuffer[i];
		}
		i++;
	}
}


void initAudio()
{
	char *devName = csd_devname(DT_AUDIO, 1); /* Get Audio Device Name */
	audioPath = open(devName, UPDAT_);        /* Open Audio Device */
	free(devName);                            /* Release memory */

	sc_atten(audioPath, 0x00000000);
	/*sc_atten(audioPath, 0x00800080);*/

	initMusic();
	initSfx();
}

void closeAudio()
{
	close(audioPath);  /* Close Audio Device */
}

void printBuffer(buffer)
u_char *buffer;
{
	int a, b;
	for (a = 0; a < 16; a++) {
		for (b = 0; b < 16; b++) {
			printf("%02X ", buffer[16 * a + b]);
		}
		printf("\n");
	}
	printf("\n");
}


void handleAudioSignal(sigCode)
{
	int i;
	switch (sigCode) {
		case SIG_AUDIO_SM:   /* printf("SIG_AUDIO_SM\n");*/ break;
		case SIG_AUDIO_RTF:  /* printf("SIG_AUDIO_RTF %x\n", musicPcb.PCB_Stat);*/ break;
		case SIG_AUDIO_SEEK:
			/*printf("SIG_SEEK\n");*/
			ss_play(musicRtf, &musicPcb);
			break;
		case SIG_AUDIO_PCL1:
			/*printf("SIG_AUDIO_PCL1\n");*/
			musicPcl1.PCL_Ctrl = 0;
			musicPcl1.PCL_Cnt  = 0;
			/*mixAudio(smMusic1_buf, smSfx_buf + (4*9216));*/

			mixSfx(smMusic1_id, smMix1_id);
			
			sm_out(audioPath, smMix1_id, &musicStat);
		break;
		case SIG_AUDIO_PCL2:
			/*printf("SIG_AUDIO_PCL2\n");*/
			musicPcl2.PCL_Ctrl = 0;
			musicPcl2.PCL_Cnt  = 0;
			
			/*mixAudio(smMusic2_buf, smSfx_buf + (4*9216));*/
			
			mixSfx(smMusic2_id, smMix2_id);
			sm_out(audioPath, smMix2_id, &musicStat);
		break;
	}
	
	
}
