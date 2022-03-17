#ifndef __GRAPHICS_H__
#define	__GRAPHICS_H__

#include "video.h"

#define TILES_X 23
#define TILES_Y 15
#define TILE_COUNT (TILES_X * TILES_Y)

#define OFFSET_X ((SCREEN_WIDTH  - TILES_X * 16) / 2)
#define OFFSET_Y ((SCREEN_HEIGHT - TILES_Y * 16) / 2)

#define HUD_WIDTH 96
#define HUD_HEIGHT 8
#define TIMER_WIDTH 56 
#define TIMER_HEIGHT 8

#define SIG_BLANK 0x0100

#define SCORE_FONT 0x7000
#define HUD_FONT   0xF000

#define GAME_FPS 50

#define ORIGIN (OFFSET_Y * SCREEN_WIDTH + OFFSET_X)

#define pixel_pos(x,y) (ORIGIN + (x) + ((y) * SCREEN_WIDTH))
#define tile_index(x,y) (((x) >> 4) + ((y) >> 4) * TILES_X)
#define tile_x(tile) ((tile % TILES_X) << 4)
#define tile_y(tile) ((tile / TILES_X) << 4)
#define tile_coord(tile) pixel_pos(tile_x(tile), tile_y(tile))
#define tile_pos(x,y) (x + y * TILES_X)

typedef struct {
	u_int spritePos[30];
	u_int spriteCount;
	u_char *videoBuffer;
} SpritePlane;

typedef struct {
	u_char *videoBuffer;
} ImagePlane;

extern unsigned char *spriteSheet;
extern unsigned char *tileSheet;
extern int curIcfA, curIcfB;
extern SpritePlane *spActive, *spDraw;
extern ImagePlane  *ipPrim, *ipSec;
extern unsigned char *hudBuffer, *timerBuffer;
extern unsigned int frameDone, frameTick;

void drawTileBuffer(position);
void drawTile(tile);
void drawTransTile(tile);

void setIcf(icfA, icfB);

#endif