#include <sysio.h>
#include <ucm.h>
#include "game.h"
#include "audio.h"

#define BOMB_FRAMES 5
#define BOMB_TICK   16
#define BOMB_USER_FRAMES 2
#define BOMB_USER_START  6

#define P_TILE_EMPTY  0 /* Tile is passable for projectiles */
#define P_TILE_LIQUID 1 /* Tile is liquid (water, lava) */
#define P_TILE_CLEAR  2 /* Tile is cleared (bush, barrel) */
#define P_TILE_BLOCK  3 /* Tile is solid */

char  BombMax  = 1;
char  BombCur  = 0;
char  BombSize = 2;
short BombType = ACT_FIRE;

int placeBomb(x, y, type, size)
	short x, y, type;
	char size;
{
	Object *bomb;
	u_short tileIndex = tile_pos(x, y);

	/* Check if we can place a bomb */
	if (BombCur >= BombMax) return 0;
	
	/* Create bomb; check if there are objects left */
	if (bomb = makeObject(OT_BOMB, 0xC0, 4, x, y)) {
		if (type != ACT_NPC) BombCur++;

		bomb->action = type | DIR_ALL | (BOMB_FRAMES * BOMB_TICK - 1);
		bomb->desc = size;
		bomb->data = tileIndex; /* Save tile index so we can mark the tile as walkable */
		bomb->data1 = gameLevel.tileMap[tileIndex];
		gameLevel.tileMap[tileIndex] = 0xFF; /* Mark tile as full but does not change the appearance */

		playSfx(SFX_FUSE);

		return 1;
	}
	else {
		return 0;
	}
}

void explodeBomb(bomb, dir)
	Object *bomb;
{
	u_short actType = bomb->action & 0xf000;
	u_short actDir  = bomb->action & 0x0f00;

	if (actType == ACT_USER || actType == ACT_FIRE) BombCur--;
	if (actType == ACT_HIDDEN) dir = DIR_NONE;
	gameLevel.tileMap[bomb->data] = (u_char)bomb->data1;
	bomb->type = OT_NONE;

	/* Check which directions to blow the bomb */
	dir = (dir ^ DIR_ALL) | actDir;

	playSfx(SFX_BOOM);

	if (dir & DIR_UP)    placeFire(bomb->x, bomb->y, DIR_UP,    bomb->desc, 1, 0);
	if (dir & DIR_DOWN)  placeFire(bomb->x, bomb->y, DIR_DOWN,  bomb->desc, 1, 0);
	if (dir & DIR_LEFT)  placeFire(bomb->x, bomb->y, DIR_LEFT,  bomb->desc, 1, 0);
	if (dir & DIR_RIGHT) placeFire(bomb->x, bomb->y, DIR_RIGHT, bomb->desc, 1, 0);
}

void stepBomb(bomb)
	Object *bomb;
{
	u_short actType = bomb->action & 0xf000;
	u_short actTime = bomb->action & 0x00ff;
	int f;
	
	if (actType == ACT_HIDDEN) {

	}
	else if (actType == ACT_USER) {
		bomb->frame = (gameTick / BOMB_TICK) % BOMB_USER_FRAMES + BOMB_USER_START;
	}
	else {
		bomb->frame = actTime / BOMB_TICK;

		if (actTime) {
			bomb->action--;
		}
		else {
			explodeBomb(bomb, DIR_NONE);
		}
	}
}

int attackTile(x, y, dir)
	short x, y, dir;
{
	u_short tileIndex = tile_pos(x, y);
	u_char  tileValue = gameLevel.tileMap[tileIndex];
	Object  *animObject;

	interactFireObject(x, y, dir);

	/* Handle Cannon */
	if (tileValue == 0x4B || tileValue == 0x4C) tileValue = 0xD8;

	if (tileValue >= 0xD0) {
		if (tileValue & 0x01) {
			gameLevel.tileMap[tileIndex] = 0x23; /* Mark tile as walkable */
			drawTile(0x23, pixel_pos(x, y), ipDraw->videoBuffer);
		}
		else {
			gameLevel.tileMap[tileIndex] = 0x00; /* Mark tile as walkable */
			drawTile(0x00, pixel_pos(x, y), ipDraw->videoBuffer);
		}
		
		backgroundChanged = 1;
		
		animObject = makeObject(OT_ANIM, (tileValue & 0xFC) + 4, 0, x, y);
		if (animObject != NULL) {
			animObject->action = ACT_ACTIVE | 11; /* Active for 12 frames */
			animObject->data = 0x0403; /* 4 frames, 3 steps/frame*/
		}
		playSfx(SFX_CRACK);
		return P_TILE_CLEAR;
	}
	else {
		return 0;
	}
}

int placeFire(x, y, dir, size, force, desc)
	short x, y, dir, size;
	char force, desc;
{
	Object *fire;

	force = force || canMoveTo(tile_pos(x, y), dir);

	if (attackTile(x, y, dir)) {
		return 0;
	}
	else if (force && (fire = makeObject(OT_FIRE, 0xB8, 0, x, y))) {
		fire->action = ACT_ACTIVE | dir;
		fire->data   = size;
		fire->desc   = desc;
		return 1;
	}
	else {
		return 0;
	}
}

int checkSpreadDir(x, y, dir)
	short x, y, dir;
{
	return gameLevel.tileMap[tile_pos(x + dir_dx(dir), y + dir_dy(dir))] == 0x00;
}

void spreadFire(fire)
	Object *fire;
{
	int i, num;
	u_short revDir;
	u_short checkDir = fire->action & 0x0F00;

	if (checkDir == DIR_RIGHT)     revDir = DIR_LEFT;
	else if (checkDir == DIR_DOWN) revDir = DIR_UP;
	else if (checkDir == DIR_LEFT) revDir = DIR_RIGHT;
	else                           revDir = DIR_DOWN;

	num = rand();

	for (i = 0; i < 4; i++) {
		num = (num + 1) % 4;
		checkDir = DIR_LEFT << num;
		if ((revDir != checkDir) && checkSpreadDir(fire->x, fire->y, checkDir)) {
			placeFire(fire->x + dir_dx(checkDir), fire->y + dir_dy(checkDir), checkDir, fire->data - 1, 1, fire->desc);
			return;
		}
	}
	placeFire(fire->x + dir_dx(revDir), fire->y + dir_dy(revDir), checkDir, fire->data - 1, 0, fire->desc);
}

void stepFire(fire)
	Object *fire;
{	
	u_short actDir  = fire->action & 0x0F00;
	u_short actTime = fire->action & 0x00FF;
	u_short fireTime = fire->data;
	
	fire->action++;
	fire->frame = actTime;

	if (actTime == 0x05 && fire->data > 0) {
		if (fire->desc) {
			spreadFire(fire, actDir);
		}
		else {
			placeFire(fire->x + dir_dx(actDir), fire->y + dir_dy(actDir), actDir, fire->data - 1, 0, 0);
		}
	}
	else if (actTime == 0x08) {
		fire->type = OT_NONE;
	}
}

void fireCannon(cannon, dir)
	Object *cannon;
	u_short dir;
{
	Object *animObject, *cannonBall;
	short x = cannon->x + dir_dx(dir);
	short y = cannon->y + dir_dy(dir);

	if (!canMoveTo(tile_pos(x, y), 0x0F00)) return;

	playSfx(SFX_BOOM);


	cannonBall = makeObject(OT_CNBALL, 0xC5, 0, x, y);
	if (cannonBall != NULL) {
		cannonBall->action = ACT_ACTIVE | dir | 0xFF;
		cannonBall->desc   = 0x21;   /* Move 1 pixel every 4 frames */
		cannonBall->data   = 0x0000; /* No animation */
	}

	animObject = makeObject(OT_ANIM, 0xDC, 0, x, y);
	if (animObject != NULL) {
		animObject->action = ACT_ACTIVE | 11; /* Active for 12 frames */
		animObject->data = 0x0403; /* 4 frames, 3 steps/frame*/
	}
}

void stepCannon(cannon)
	Object *cannon;
{
	u_short actDir  = cannon->action & 0x0F00;
	u_short actTime = cannon->action & 0x00FF;

	if (!actTime) {
		fireCannon(cannon, actDir);
		cannon->action |= cannon->data;
	}
	else {
		cannon->action--;
	}
}

void stepProjectile(object)
	Object *object;
{
	u_short actDir  = object->action & 0x0F00;
	short x = object->x + dir_dx(actDir);
	short y = object->y + dir_dy(actDir);
	stepAnim(object);
	if (!((x | y) & 0x000f)) {
		if (attackTile(x, y, actDir) || !canMoveTo(tile_pos(x, y), 0x2000)) {
			object->type = OT_NONE;
		}
	}
}

void stepTorch(torch)
	Object *torch;
{
	torch->frame = (gameTick >> 2) % 4;
	if (torch->frame == 3) torch->frame = 1;
}

void activateTorch(torch)
	Object *torch;
{
	u_short actDir  = torch->action & 0x0F00;
	u_short actTime = torch->action & 0x00FF;

	if (actTime) {
		placeFire(torch->x + dir_dx(actDir), torch->y + dir_dy(actDir), actDir, 0xff, 1, 1);
		torch->action--;
	}
}