#ifndef __GRAPHICS_H__
#define	__GRAPHICS_H__

#include "video.h"

#define TILES_X 23
#define TILES_Y 15

#define OFFSET_X (SCREEN_WIDTH  - TILES_X * 16) / 2
#define OFFSET_Y (SCREEN_HEIGHT - TILES_Y * 16) / 2

#define HUD_WIDTH 64
#define HUD_HEIGHT 8
#define HGFX_START 0xF080

#define ORIGIN (OFFSET_Y * SCREEN_WIDTH + OFFSET_X)

#define pixel_pos(x,y) ORIGIN + (x) + ((y) * SCREEN_WIDTH)
#define tile_pos(x,y) ((x) >> 4) + ((y) >> 4) * TILES_X
#define tile_x(tile) ((tile % TILES_X) << 4)
#define tile_y(tile) ((tile / TILES_X) << 4)
#define tile_coord(tile) pixel_pos(tile_x(tile), tile_y(tile))

typedef struct {
	u_int spritePos[30];
	u_int spriteCount;
	u_char *videoBuffer;
} SpritePlane;

typedef struct {
	u_char *videoBuffer;
} ImagePlane;

extern unsigned char *spriteSheet;
extern int curICF;
extern SpritePlane *spActive, *spDraw;
extern ImagePlane  *ipActive, *ipDraw;
extern unsigned char *hudBuffer;

#endif