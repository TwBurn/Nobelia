#ifndef __GAME_H__
#define	__GAME_H__

#include "graphics.h"

#ifndef NULL
#define NULL 0
#endif

#define OT_NONE    0x00
#define OT_PLAYER  0x01
#define OT_BOMB    0x02
#define OT_FIRE    0x03
#define OT_MAGIC   0x04
#define OT_SWITCH  0x10
#define OT_COIN    0x11
#define OT_HEART   0x12
#define OT_CHEST   0x20
#define OT_TRAP    0x30
#define OT_CANNON  0x40
#define OT_TORCH   0x41
#define OT_CNBALL  0x48
#define OT_SKELET  0x80
#define OT_BONE    0x81
#define OT_ANIM    0xF0

#define ACT_HIDDEN 0x0000
#define ACT_IDLE   0x1000
#define ACT_ACTIVE 0x2000
#define ACT_MOVE   0x3000
#define ACT_FIRE   0x4000
#define ACT_USER   0xA000
#define ACT_NPC    0xB000
#define ACT_DEAD   0xD000

#define DIR_NONE   0x0000
#define DIR_ALL    0x0F00
#define DIR_LEFT   0x0100
#define DIR_RIGHT  0x0200
#define DIR_UP     0x0400
#define DIR_DOWN   0x0800

#define START_LEVEL 1
#define START_BMAX  0
#define START_BSIZE 1
#define START_BTYPE ACT_FIRE
#define LEVEL_COUNT 4
#define OBJ_COUNT   32

#define dir_dx(dir) (dir==DIR_LEFT?-16:(dir==DIR_RIGHT?16:0))
#define dir_dy(dir) (dir==DIR_UP?-16:(dir==DIR_DOWN?16:0))
#define dir_tile(dir) (dir==DIR_LEFT?-1:(dir==DIR_RIGHT?1:(dir==DIR_UP?-TILES_X:(dir==DIR_DOWN?TILES_X:0))))
#define pix_dx(dir) (dir==DIR_LEFT?-1:(dir==DIR_RIGHT?1:0))
#define pix_dy(dir) (dir==DIR_UP?-1:(dir==DIR_DOWN?1:0))

typedef struct Object
{
	u_char type;
	u_char sprite;
	u_char frame;
	u_char desc;
	u_short action;
	u_short data;
	u_short data1;
	u_short data2;
	short x;
	short y;
} Object;

typedef struct Level {
	u_char levelName[7];
	u_char tileMap[23 * 15];
	Object objects[OBJ_COUNT];
} Level;

extern char  BombMax;  /* Number of bombs */
extern char  BombCur;  /* Number of placed bombs */
extern char  BombSize; /* Explosion Radius */
extern short BombType; /* Bomb Type */

extern u_int gameTick;

extern Level gameLevel;
extern u_char backgroundChanged;

Object* makeObject(type, sprite, frame, x, y);
Object* findObject(x, y);

char canMoveTo(tile, dir);
/*u_short canMove(fromTile, dir);*/

#endif