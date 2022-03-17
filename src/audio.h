#ifndef __AUDIO_H__
#define	__AUDIO_H__

/*Note that there are actually 26 entries; due to alignment we need to reserve a bit more space*/

#define SFX_SIZE 106496 /* Smallest multiple of 2048 greater or equal to SFX.BIN */

#define SFX_DEF(start, size) start, (start + size - 1)
#define SFX_NONE   SFX_DEF( 0, 1)
#define SFX_STEP   SFX_DEF( 1, 4)
#define SFX_CRACK  SFX_DEF( 5, 3)
#define SFX_FUSE   SFX_DEF( 8, 8)
#define SFX_TPORT  SFX_DEF(16, 7)
#define SFX_BOOM   SFX_DEF(23, 4)
#define SFX_SWITCH SFX_DEF(27, 3)
#define SFX_ENHIT  SFX_DEF(30, 3)
#define SFX_PLHIT  SFX_DEF(33, 2)
#define SFX_DEAD   SFX_DEF(35, 2)
#define SFX_POWER  SFX_DEF(37, 6)
#define SFX_MZFAIL SFX_DEF(42, 3)

#define SFX_LIMIT 3  /* Maximum number of sound effects active at the same time */
#define SFX_IGNORE 4 /* Low priority sound, only play when nothing else is playing */

#define MUSIC_STOP  0xFF
#define MUSIC_PLAY  0x04

typedef struct {
	int id;
	char *buffer;
} SoundMap;

typedef struct {
	u_short current;
	u_short last;
} SoundEffect;

void playSfx(start, size);
void stopMusic();
void playMusic(channel);

extern int curChannel;

#endif