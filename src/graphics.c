#include <sysio.h>
#include <ucm.h>
#include <stdio.h>
#include <memory.h>
#include "video.h"
#include "graphics.h"

SpritePlane spA1, spA2;
SpritePlane *spActive = &spA1, *spDraw = &spA2;

ImagePlane ipB1, ipB2;
ImagePlane *ipActive = &ipB1, *ipDraw = &ipB2;

u_char *spriteSheet;
int curICF = ICF_MAX;

u_char *hudBuffer;

void fillBuffer(buffer, data, size)
	u_int *buffer, data, size;
{
	int i;
	for (i = 0; i < (size / 4); i++) {
		buffer[i] = data;
	}
}

void fillVideoBuffer(videoBuffer, data)
	u_int *videoBuffer, data;
{
	fillBuffer(videoBuffer, data, VBUFFER_SIZE);
}

void createVideoBuffers()
{
	spA1.videoBuffer = (u_char*)srqcmem(VBUFFER_SIZE, VIDEO1);
	spA2.videoBuffer = (u_char*)srqcmem(VBUFFER_SIZE, VIDEO1);

	ipB1.videoBuffer = (u_char*)srqcmem(VBUFFER_SIZE, VIDEO2);
	ipB2.videoBuffer = (u_char*)srqcmem(VBUFFER_SIZE, VIDEO2);
	
	fillVideoBuffer(spA1.videoBuffer, 0);
	fillVideoBuffer(spA2.videoBuffer, 0);
	fillVideoBuffer(ipB1.videoBuffer, 0);
	fillVideoBuffer(ipB2.videoBuffer, 0);

	hudBuffer = spA1.videoBuffer + (SCREEN_WIDTH * SCREEN_HEIGHT);

	dc_wrli(videoPath, lctA, 1, 0, cp_dadr((int)spA1.videoBuffer + pixelStart));
	dc_wrli(videoPath, lctA, 1, 1, cp_sig());
	dc_wrli(videoPath, lctB, 1, 0, cp_dadr((int)ipB1.videoBuffer + pixelStart));
	dc_exec(videoPath, fctA, fctB);
}

void loadPalette(plane, filename)
    int plane;
    u_char *filename;
{
    FILE *file = fopen(filename, "r");
	fread(fctBuffer, sizeof(int), 130, file);
	fclose(file);
	dc_wrfct(videoPath, (plane == PA) ? fctA : fctB, 0, 130, fctBuffer);
	dc_exec(videoPath, fctA, fctB);
}

void loadImage(file, videoBuffer)
    int file;
	u_char *videoBuffer;
{
	read(file, videoBuffer, VBUFFER_SIZE);
}

void loadSpriteSheet(file)
	int file;
{
	read(file, spriteSheet, 256*256);
}

void drawTile(tile, position, videoBuffer) 
	u_char tile;
	u_int position;
	u_char *videoBuffer;
{
	u_char* src = spriteSheet + ((tile & 0x0f) << 4) + ((tile & 0xf0) << 8);
	u_char* dst = videoBuffer + position;

	memcpy(dst +  0 * SCREEN_WIDTH, src +  0 * 256, 16);
	memcpy(dst +  1 * SCREEN_WIDTH, src +  1 * 256, 16);
	memcpy(dst +  2 * SCREEN_WIDTH, src +  2 * 256, 16);
	memcpy(dst +  3 * SCREEN_WIDTH, src +  3 * 256, 16);
	memcpy(dst +  4 * SCREEN_WIDTH, src +  4 * 256, 16);
	memcpy(dst +  5 * SCREEN_WIDTH, src +  5 * 256, 16);
	memcpy(dst +  6 * SCREEN_WIDTH, src +  6 * 256, 16);
	memcpy(dst +  7 * SCREEN_WIDTH, src +  7 * 256, 16);
	memcpy(dst +  8 * SCREEN_WIDTH, src +  8 * 256, 16);
	memcpy(dst +  9 * SCREEN_WIDTH, src +  9 * 256, 16);
	memcpy(dst + 10 * SCREEN_WIDTH, src + 10 * 256, 16);
	memcpy(dst + 11 * SCREEN_WIDTH, src + 11 * 256, 16);
	memcpy(dst + 12 * SCREEN_WIDTH, src + 12 * 256, 16);
	memcpy(dst + 13 * SCREEN_WIDTH, src + 13 * 256, 16);
	memcpy(dst + 14 * SCREEN_WIDTH, src + 14 * 256, 16);
	memcpy(dst + 15 * SCREEN_WIDTH, src + 15 * 256, 16);
}

void drawHudTile(tile, x)
	char tile, x;
{
	u_char* src = spriteSheet + HGFX_START + (tile << 3);
	u_char* dst = hudBuffer + (x << 3);

	memcpy(dst +  0 * HUD_WIDTH, src +  0 * 256, 8);
	memcpy(dst +  1 * HUD_WIDTH, src +  1 * 256, 8);
	memcpy(dst +  2 * HUD_WIDTH, src +  2 * 256, 8);
	memcpy(dst +  3 * HUD_WIDTH, src +  3 * 256, 8);
	memcpy(dst +  4 * HUD_WIDTH, src +  4 * 256, 8);
	memcpy(dst +  5 * HUD_WIDTH, src +  5 * 256, 8);
	memcpy(dst +  6 * HUD_WIDTH, src +  6 * 256, 8);
	memcpy(dst +  7 * HUD_WIDTH, src +  7 * 256, 8);
}

void clearHud()
{
	fillBuffer(hudBuffer, 0, HUD_WIDTH * HUD_HEIGHT);
}

void drawHud(x, y, videoBuffer)
	short x, y;
	u_char *videoBuffer;
{
	u_char* dst = videoBuffer + pixel_pos(x, y);

	memcpy(dst + 0 * SCREEN_WIDTH, hudBuffer + 0 * HUD_WIDTH, HUD_WIDTH);
	memcpy(dst + 1 * SCREEN_WIDTH, hudBuffer + 1 * HUD_WIDTH, HUD_WIDTH);
	memcpy(dst + 2 * SCREEN_WIDTH, hudBuffer + 2 * HUD_WIDTH, HUD_WIDTH);
	memcpy(dst + 3 * SCREEN_WIDTH, hudBuffer + 3 * HUD_WIDTH, HUD_WIDTH);
	memcpy(dst + 4 * SCREEN_WIDTH, hudBuffer + 4 * HUD_WIDTH, HUD_WIDTH);
	memcpy(dst + 5 * SCREEN_WIDTH, hudBuffer + 5 * HUD_WIDTH, HUD_WIDTH);
	memcpy(dst + 6 * SCREEN_WIDTH, hudBuffer + 6 * HUD_WIDTH, HUD_WIDTH);
	memcpy(dst + 7 * SCREEN_WIDTH, hudBuffer + 7 * HUD_WIDTH, HUD_WIDTH);
}

void loadTilemap(videoBuffer, tilemap)
	u_char *videoBuffer;
	u_char *tilemap;
{
	u_short x;
	u_short y;
	u_short i = 0;
	u_int origin;

	for (y = 0; y < TILES_Y; y++) {
		for (x = 0; x < TILES_X; x++) {
			drawTile(
				tilemap[i++],
				pixel_pos(x << 4, y << 4),
				videoBuffer
			);
		}
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
		drawTile(
			0xFF,
			spDraw->spritePos[--spDraw->spriteCount],
			spDraw->videoBuffer
		);
	}
}

void swapSpriteBuffer()
{
	SpritePlane* temp = spDraw;
	spDraw = spActive;
	spActive = temp;
	dc_wrli(videoPath, lctA, 1, 0, cp_dadr((int)spActive->videoBuffer + pixelStart));
}

void drawSprite(tile, x, y)
{
	u_int position = pixel_pos(x, y);
	spDraw->spritePos[spDraw->spriteCount++] = position;
	drawTile(tile, position, spDraw->videoBuffer);
}

void swapImageBuffer()
{
	ImagePlane* temp = ipDraw;
	ipDraw = ipActive;
	ipActive = temp;
	dc_wrli(videoPath, lctB, 1, 0, cp_dadr((int)ipActive->videoBuffer + pixelStart));
}

void setICF(value)
	int value;
{
	if      (value > ICF_MAX) curICF = ICF_MAX;
	else if (value < ICF_MIN) curICF = ICF_MIN;
	else                      curICF = value;

	dc_wrli(videoPath, lctA, 1, 7, cp_icf(PA, curICF));
	dc_wrli(videoPath, lctB, 1, 7, cp_icf(PB, curICF));
}

void initGraphics()
{
	createVideoBuffers();
	loadPalette(PA, "PLANE_A.PAL");
	loadPalette(PB, "PLANE_B.PAL");
	spriteSheet = (u_char*)srqcmem(256 * 256, VIDEO2);
}