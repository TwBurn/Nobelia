#ifndef __AUDIO_H__
#define	__AUDIO_H__

/*Note that there are actually 26 entries; due to alignment we need to reserve a bit more space*/

#define SFX_SIZE 61440 /* Smallest multiple of 2048 greater or equal to SFX.BIN */
#define SFX_ENTRIES 27 /* SFX_SIZE divided by 2304, rounded up */


#define SFX_NONE 0, 1
#define SFX_STEP 1, 4
#define SFX_CRACK 5, 3
#define SFX_FUSE 8, 8
#define SFX_BOOM 16, 4
#define SFX_POWER 20, 6

void playSfx(start, size);

#endif