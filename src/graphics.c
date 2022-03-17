#include <sysio.h>
#include <ucm.h>
#include <stdio.h>
#include <memory.h>
#include "video.h"
#include "graphics.h"

#define INTRO_SIZE 4096
#define DISC_ERROR_SIZE 10240

u_int frameDone = 0, frameTick = 0;

SpritePlane spA1, spA2;
SpritePlane *spActive = &spA1, *spDraw = &spA2;

ImagePlane ipB1, ipB2;
ImagePlane *ipPrim = &ipB1;
ImagePlane *ipSec  = &ipB2;

u_char *spriteSheet;
u_char *tileSheet;
int curIcfA = ICF_MAX;
int curIcfB = ICF_MAX;

u_char *hudBuffer;
u_char *timerBuffer;
u_char *tileBuffer;

u_char* rleBuffer;

u_char shadowColors[] = {0,1,1,1,2,3,4,5,6,7,8,13,14,15,16,17,1,1,1,1,18,19,20,39,36,27,4,3,30,31,1,1,1,1,32,33,34,35,41,42,43,33,1,1,1,1,44,45,46,51,52,43,43,33,1,33,54,55,56,61,62,63,64,33,1};
/*u_char shadowColors[] = {0,1,1,2,3,4,5,6,7,8,9,12,13,14,15,16,17,1,1,18,19,20,21,38,25,26,27,6,29,30,31,1,1,1,33,34,35,36,39,40,41,42,43,33,1,44,45,46,47,50,51,42,42,43,43,54,55,56,57,60,61,62,63,64,33};*/

void fillBuffer(buffer, data, size)
	register u_int *buffer;
	register u_int data, size;
{
	int i;
	size = size >> 2;
	for (i = 0; i < size; i++) {
		*buffer++ = data;
	}
}

void fillVideoBuffer(videoBuffer, data)
	register u_int *videoBuffer;
	u_int data;
{
	fillBuffer(videoBuffer, data, VBUFFER_SIZE);
}

void createVideoBuffers()
{
	setIcf(ICF_MIN, ICF_MIN);
	spA1.videoBuffer = (u_char*)srqcmem(VBUFFER_SIZE, VIDEO1);
	spA2.videoBuffer = (u_char*)srqcmem(VBUFFER_SIZE, VIDEO1);

	ipB1.videoBuffer = (u_char*)srqcmem(VBUFFER_SIZE, VIDEO2);
	ipB2.videoBuffer = (u_char*)srqcmem(VBUFFER_SIZE, VIDEO2);
	
	fillVideoBuffer(spA1.videoBuffer, 0);
	fillVideoBuffer(spA2.videoBuffer, 0);
	fillVideoBuffer(ipB1.videoBuffer, 0);
		
	hudBuffer   = spA1.videoBuffer + SCREEN_SIZE;
	timerBuffer = spA2.videoBuffer + SCREEN_SIZE;
	tileBuffer  = ipB1.videoBuffer + SCREEN_SIZE;

	dc_wrli(videoPath, lctA, 0, 1, cp_dadr((int)spA1.videoBuffer + pixelStart));
	dc_wrli(videoPath, lctA, 0, 0, cp_sig());
	dc_wrli(videoPath, lctB, 0, 1, cp_dadr((int)ipB1.videoBuffer + pixelStart));
}

void readPalette(plane, filename)
    int plane;
    u_char *filename;
{
    FILE *file = fopen(filename, "r");
	fread(fctBuffer, sizeof(int), 130, file);
	fclose(file);
	dc_wrfct(videoPath, (plane == PA) ? fctA : fctB, FCT_PAL_START, 130, fctBuffer);
}

int readImage(file, videoBuffer)
    int file;
	u_char *videoBuffer;
{
	return read(file, videoBuffer, VBUFFER_SIZE);
}


int readTileSheet(file)
	int file;
{
	return read(file, tileSheet, 256*256);
}

int readSpriteSheet(file)
	int file;
{
	return read(file, spriteSheet, 256*256);
}

int readIntroData(file)
{
	return read(file, rleBuffer, INTRO_SIZE);
}

int readDiscError(file)
{
	return read(file, rleBuffer, DISC_ERROR_SIZE);
}


void clearTileBuffer(background)
	register int background;
{
	register u_int* src = (u_int*)tileBuffer;
	register int i;

	for (i = 0; i < 16; i++) {
		/* Clear with background */
		*src++=background; *src++=background; *src++=background; *src++=background;
	}
}

void drawTileBuffer(position, background)
	int position;
	register int background;
{
	register u_int* src = (u_int*)tileBuffer;
	register u_int* dst = (u_int*)(ipPrim->videoBuffer + position);

	register int i;

	for (i = 0; i < 16; i++) {
		/* Copy 16 bytes, clear with background */
		*dst++=*src; *src++=background;
		*dst++=*src; *src++=background;
		*dst++=*src; *src++=background;
		*dst++=*src; *src++=background;
		dst += (SCREEN_WIDTH - 16) >> 2;
	}
}

void drawTile(tile) 
	u_char tile;
{
	register u_int* src = (u_int*)(tileSheet + ((tile & 0x0f) << 4) + ((tile & 0xf0) << 8));
	register u_int* dst = (u_int*)tileBuffer;

	int i;

	for (i = 0; i < 16; i++) {
		/* Copy 16 bytes */
		*dst++=*src++; *dst++=*src++; *dst++=*src++; *dst++=*src++;	
		src += (256 - 16) >> 2;
	}
}

void drawTransTile(tile) 
	u_char tile;
{
	register u_char* src = tileSheet + ((tile & 0x0f) << 4) + ((tile & 0xf0) << 8);
	register u_char* dst = tileBuffer;
	register u_char tmp;
	int y;
	
	
	for (y = 0; y < 16; y++) {
		tmp = *src++; if (tmp == 1) { *dst = shadowColors[*dst]; } else if (tmp) { *dst = tmp; } dst++;
		tmp = *src++; if (tmp == 1) { *dst = shadowColors[*dst]; } else if (tmp) { *dst = tmp; } dst++;
		tmp = *src++; if (tmp == 1) { *dst = shadowColors[*dst]; } else if (tmp) { *dst = tmp; } dst++;
		tmp = *src++; if (tmp == 1) { *dst = shadowColors[*dst]; } else if (tmp) { *dst = tmp; } dst++;

		tmp = *src++; if (tmp == 1) { *dst = shadowColors[*dst]; } else if (tmp) { *dst = tmp; } dst++;
		tmp = *src++; if (tmp == 1) { *dst = shadowColors[*dst]; } else if (tmp) { *dst = tmp; } dst++;
		tmp = *src++; if (tmp == 1) { *dst = shadowColors[*dst]; } else if (tmp) { *dst = tmp; } dst++;
		tmp = *src++; if (tmp == 1) { *dst = shadowColors[*dst]; } else if (tmp) { *dst = tmp; } dst++;

		tmp = *src++; if (tmp == 1) { *dst = shadowColors[*dst]; } else if (tmp) { *dst = tmp; } dst++;
		tmp = *src++; if (tmp == 1) { *dst = shadowColors[*dst]; } else if (tmp) { *dst = tmp; } dst++;
		tmp = *src++; if (tmp == 1) { *dst = shadowColors[*dst]; } else if (tmp) { *dst = tmp; } dst++;
		tmp = *src++; if (tmp == 1) { *dst = shadowColors[*dst]; } else if (tmp) { *dst = tmp; } dst++;

		tmp = *src++; if (tmp == 1) { *dst = shadowColors[*dst]; } else if (tmp) { *dst = tmp; } dst++;
		tmp = *src++; if (tmp == 1) { *dst = shadowColors[*dst]; } else if (tmp) { *dst = tmp; } dst++;
		tmp = *src++; if (tmp == 1) { *dst = shadowColors[*dst]; } else if (tmp) { *dst = tmp; } dst++;
		tmp = *src++; if (tmp == 1) { *dst = shadowColors[*dst]; } else if (tmp) { *dst = tmp; } dst++;

		src += 256 - 16;
	}
}

void drawSpriteTrans(tile, position, videoBuffer)
	u_char tile;
	u_int position;
	u_char *videoBuffer;
{
	register u_char* src = spriteSheet + ((tile & 0x0f) << 4) + ((tile & 0xf0) << 8);
	register u_char* dst = videoBuffer + position;
	register u_char tmp;
	register int y;
	
	for (y = 0; y < 16; y++) {
		tmp = *src++; if (tmp) { *dst = tmp; } dst++;
		tmp = *src++; if (tmp) { *dst = tmp; } dst++;
		tmp = *src++; if (tmp) { *dst = tmp; } dst++;
		tmp = *src++; if (tmp) { *dst = tmp; } dst++;

		tmp = *src++; if (tmp) { *dst = tmp; } dst++;
		tmp = *src++; if (tmp) { *dst = tmp; } dst++;
		tmp = *src++; if (tmp) { *dst = tmp; } dst++;
		tmp = *src++; if (tmp) { *dst = tmp; } dst++;

		tmp = *src++; if (tmp) { *dst = tmp; } dst++;
		tmp = *src++; if (tmp) { *dst = tmp; } dst++;
		tmp = *src++; if (tmp) { *dst = tmp; } dst++;
		tmp = *src++; if (tmp) { *dst = tmp; } dst++;

		tmp = *src++; if (tmp) { *dst = tmp; } dst++;
		tmp = *src++; if (tmp) { *dst = tmp; } dst++;
		tmp = *src++; if (tmp) { *dst = tmp; } dst++;
		tmp = *src++; if (tmp) { *dst = tmp; } dst++;

		dst += SCREEN_WIDTH - 16;
		src += 256 - 16;
	}
}

void drawSpriteTile(tile, position, videoBuffer)
	u_char tile;
	u_int position;
	u_char *videoBuffer;
{
	register u_char* src = spriteSheet + ((tile & 0x0f) << 4) + ((tile & 0xf0) << 8);
	register u_char* dst = videoBuffer + position;
	register int i;
	
	for (i = 0; i < 16; i++) {
		/* Copy 16 bytes */
		*dst++=*src++; *dst++=*src++; *dst++=*src++; *dst++=*src++;
		*dst++=*src++; *dst++=*src++; *dst++=*src++; *dst++=*src++;
		*dst++=*src++; *dst++=*src++; *dst++=*src++; *dst++=*src++;
		*dst++=*src++; *dst++=*src++; *dst++=*src++; *dst++=*src++;

		dst += SCREEN_WIDTH - 16;
		src += 256 - 16;
	}
}

void clearSpriteTile(position, videoBuffer)
	u_int position;
	u_char *videoBuffer;
{
	register u_char* dst = videoBuffer + position;
	register int i;
	
	for (i = 0; i < 16; i++) {
		/* Copy 16 bytes */
		*dst++=0; *dst++=0; *dst++=0; *dst++=0;
		*dst++=0; *dst++=0; *dst++=0; *dst++=0;
		*dst++=0; *dst++=0; *dst++=0; *dst++=0;
		*dst++=0; *dst++=0; *dst++=0; *dst++=0;

		dst += SCREEN_WIDTH - 16;
	}
}

void drawTimerTile(tile, x)
	char tile, x;
{
	register u_int* src = (u_int*)(spriteSheet + HUD_FONT + ((tile & 0x20) << 6) + ((tile & 0x1f) << 3));
	register u_int* dst = (u_int*)(timerBuffer + (x << 3));

	int i;

	for (i = 0; i < 8; i++) {
		/* Copy 8 bytes */
		*dst++=*src++; *dst++=*src++;

		dst += (TIMER_WIDTH - 8) >> 2;
		src += (256 - 8) >> 2;
	}
}

void drawTimerTime(time)
	u_int time;
{
	u_int res;
	/* Strip out frames */
	time = time / GAME_FPS;
	
	/* Seconds */
	res = time % 60; time = time / 60;
	drawTimerTile(0x20 + res % 10, 6);
	drawTimerTile(0x20 + res / 10, 5);
	drawTimerTile(0x2b, 4);
	/* Minutes */
	res = time % 60; time = time / 60;
	drawTimerTile(0x20 + res % 10, 3);
	drawTimerTile(0x20 + res / 10, 2);
	drawTimerTile(0x2b, 1);
	/* Hours */
	drawTimerTile(0x20 + time % 10, 0);
}

void clearTimer()
{
	fillBuffer(timerBuffer, 0, TIMER_WIDTH * TIMER_HEIGHT);
}

void drawTimer(x, y, videoBuffer)
	short x, y;
	u_char *videoBuffer;
{
	register u_int* src = (u_int*)timerBuffer;
	register u_int* dst = (u_int*)(videoBuffer + pixel_pos(x, y));

	int i;

	for (i = 0; i < 8; i++) {

		/* Copy 56 bytes */
		*dst++=*src++; *dst++=*src++; *dst++=*src++; *dst++=*src++;
		*dst++=*src++; *dst++=*src++; *dst++=*src++; *dst++=*src++;
		*dst++=*src++; *dst++=*src++; *dst++=*src++; *dst++=*src++;
		*dst++=*src++; *dst++=*src++; 

		dst += (SCREEN_WIDTH - TIMER_WIDTH) >> 2;
	}
}

void drawHudTile(tile, x)
	char tile, x;
{
	register u_int* src = (u_int*)(spriteSheet + HUD_FONT + ((tile & 0x20) << 6) + ((tile & 0x1f) << 3));
	register u_int* dst = (u_int*)(hudBuffer + (x << 3));

	int i;

	for (i = 0; i < 8; i++) {
		/* Copy 8 bytes */
		*dst++=*src++; *dst++=*src++;

		dst += (HUD_WIDTH - 8) >> 2;
		src += (256 - 8) >> 2;
	}
}

void clearHud()
{
	fillBuffer(hudBuffer, 0, HUD_WIDTH * HUD_HEIGHT);
}

void drawHud(x, y, videoBuffer)
	short x, y;
	u_char *videoBuffer;
{
	register u_int* src = (u_int*)hudBuffer;
	register u_int* dst = (u_int*)(videoBuffer + pixel_pos(x, y));

	int i;

	for (i = 0; i < 8; i++) {

		/* Copy 96 bytes */
		*dst++=*src++; *dst++=*src++; *dst++=*src++; *dst++=*src++;
		*dst++=*src++; *dst++=*src++; *dst++=*src++; *dst++=*src++;
		*dst++=*src++; *dst++=*src++; *dst++=*src++; *dst++=*src++;
		*dst++=*src++; *dst++=*src++; *dst++=*src++; *dst++=*src++;
		*dst++=*src++; *dst++=*src++; *dst++=*src++; *dst++=*src++;
		*dst++=*src++; *dst++=*src++; *dst++=*src++; *dst++=*src++;

		dst += (SCREEN_WIDTH - HUD_WIDTH) >> 2;
	}
}

void copyRect(sourceBuffer, targetBuffer, x, y, width, height, sourceWidth)
	u_char *sourceBuffer, *targetBuffer;
	u_short x, y, width, height, sourceWidth;
{
	register u_char* dst = targetBuffer + y * SCREEN_WIDTH + x;
	register u_char* src = sourceBuffer;
	register u_short h, w;
	register u_char tmp;

	for (h = 0; h < height; h++) {
		for (w = 0; w < width; w++) {
			tmp = *src++; if (tmp) { *dst = tmp; } dst++;
		}
		dst += SCREEN_WIDTH - width;
		src += sourceWidth - width;
	}
}

void clearRect(videoBuffer, x, y, width, height, color)
	u_char *videoBuffer;
	u_short x, y, width, height;
	u_char color;
{
	register u_int value = (color << 24) | (color << 16) | (color << 8) | color;
	register u_int* dst = (u_int*)(videoBuffer + y * SCREEN_WIDTH + x);
	register u_short h, w;

	width >>= 2;

	for (h = 0; h < height; h++) {
		for (w = 0; w < width; w++) *dst++ = value;
		dst += (SCREEN_WIDTH >> 2) - width;
	}

}


void clearSpriteBuffers()
{
	fillVideoBuffer(spA1.videoBuffer, 0);
	fillVideoBuffer(spA2.videoBuffer, 0);
	spA1.spriteCount = 0;
	spA2.spriteCount = 0;
}

void clearSprites()
{
	while (spDraw->spriteCount) {
		clearSpriteTile(spDraw->spritePos[--spDraw->spriteCount], spDraw->videoBuffer);
	}
}

void swapSpriteBuffer()
{
	SpritePlane* temp = spDraw;
	spDraw = spActive;
	spActive = temp;
	dc_wrli(videoPath, lctA, 0, 1, cp_dadr((int)spActive->videoBuffer + pixelStart));
}

void showPrimaryImageBuffer()
{
	dc_wrli(videoPath, lctB, 0, 1, cp_dadr((int)ipPrim->videoBuffer + pixelStart));
}
void showSecondaryImageBuffer()
{
	dc_wrli(videoPath, lctB, 0, 1, cp_dadr((int)ipSec->videoBuffer + pixelStart));
}

void drawSprite(tile, x, y)
	register u_char tile;
	register u_short x, y;
{
	u_int position = pixel_pos(x, y);
	spDraw->spritePos[spDraw->spriteCount++] = position;
	drawSpriteTrans(tile, position, spDraw->videoBuffer);
}

void decodeClut(source, target)
	register u_char* source;
	register u_char* target;
{
	register int x, y;
	register u_char count, value;

	for (y = 0; y < SCREEN_HEIGHT; y++) {
		x = SCREEN_WIDTH;

		while (x) {
			value = *source++;

			if (value & 0x80) {
				value &= 0x7F;
				count = *source++;
				if (count) {
					while(count--) {
						*target++ = value; x--;
					}
				}
				else {
					while (x) {
						*target++ = value; x--;
					}
				}
			}
			else {
				*target++ = value; x--;
			}
		}
	}
}

void decodeIntro() {
	decodeClut(rleBuffer, ipSec->videoBuffer);
}

void drawDiscError()
{
	dc_wrli(videoPath, lctA, 1, 7, cp_icf(PA, ICF_MIN));
	dc_wrli(videoPath, lctB, 1, 7, cp_icf(PB, ICF_MIN));

	decodeClut(rleBuffer, spActive->videoBuffer);
	showPrimaryImageBuffer();

	dc_wrli(videoPath, lctA, 1, 7, cp_icf(PA, ICF_MAX));
}

void clearDiscError()
{
	dc_wrli(videoPath, lctA, 1, 7, cp_icf(PA, ICF_MIN));
	dc_wrli(videoPath, lctB, 1, 7, cp_icf(PB, ICF_MIN));

	fillVideoBuffer(spActive->videoBuffer, 0);

	setIcf(curIcfA, curIcfB);
}

void setIcf(icfA, icfB)
	register int icfA, icfB;
{
	curIcfA = icfA > ICF_MAX ? ICF_MAX : (icfA < ICF_MIN ? ICF_MIN : icfA);
	curIcfB = icfB > ICF_MAX ? ICF_MAX : (icfB < ICF_MIN ? ICF_MIN : icfB);

	dc_wrli(videoPath, lctA, 1, 7, cp_icf(PA, curIcfA));
	dc_wrli(videoPath, lctB, 1, 7, cp_icf(PB, curIcfB));
}

void initGraphics()
{
	createVideoBuffers();
	readPalette(PA, "PLANE_A.PAL");
	readPalette(PB, "PLANE_B.PAL");
	spriteSheet = (u_char*)srqcmem(256 * 256, VIDEO2);
	tileSheet   = (u_char*)srqcmem(256 * 256, VIDEO1);
	rleBuffer   = (u_char*)srqcmem(DISC_ERROR_SIZE, VIDEO1);
}

void handleVideoSignal(sigCode)
	int sigCode;
{
	if (sigCode == SIG_BLANK) {
		frameDone = 1;
		frameTick++;
	}
}