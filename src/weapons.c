#include <sysio.h>
#include <ucm.h>
#include "game.h"
#include "graphics.h"
#include "audio.h"

#define BOMB_SPRITE 0xD0
#define BOMB_FRAMES 5
#define BOMB_TICK   16
#define BOMB_USER_FRAMES 2
#define BOMB_USER_START  6

#define FIRE_SPRITE 0xE8
#define BALL_SPRITE 0xD5
#define BALL_MASK   0x20

#define P_TILE_EMPTY  0 /* Tile is passable for projectiles */
#define P_TILE_LIQUID 1 /* Tile is liquid (water, lava) */
#define P_TILE_CLEAR  2 /* Tile is cleared (bush, barrel) */
#define P_TILE_BLOCK  3 /* Tile is solid */

/*
	Check if fire can spread to the tile with index '''tile''' using direction '''dir'''
*/
u_char canSpreadTo(tile, dir)
	register u_short tile, dir;
{
	register u_char value = gameLevel.tileMap[tile].move;
	register u_char check = dir >> 8;

	if (value == 0) {
		return 1; /* Special Case for Switches */
	}
	else if (value & 0x10) {
		return 1;
	}
	else if (value & 0x40) {
		return 0xFF;
	}
	else {
		return value & check;
	}
}

int placeFire(x, y, dir, size, force, desc)
	register u_short x, y, dir, size;
	register u_char force, desc;
{
	register Sprite *fire;
	register u_char place = canSpreadTo(tile_index(x, y), force ? DIR_ALL : dir);
	/* force = force || canSpreadTo(tile_index(x, y), dir);*/

	if (place == 0xFF) {
		return 0; /* Tile blocked for fire */
	}
	else if (attackTile(x, y, dir, AT_FIRE, fire->desc)) {
		return 0;
	}
	else if (place && (fire = makeSprite(ST_FIRE, FIRE_SPRITE, 0, x, y))) {
		fire->action = ACT_BURN | dir;
		fire->data   = size;
		fire->desc   = desc;
		return 1;
	}
	else {
		return 0;
	}
}

void placeFires(x, y, dir, size, force, desc)
	register u_short x, y, dir, size;
	register u_char force, desc;
{
	if (dir & DIR_UP)    placeFire(x, y, DIR_UP,    size, force, desc);
	if (dir & DIR_DOWN)  placeFire(x, y, DIR_DOWN,  size, force, desc);
	if (dir & DIR_LEFT)  placeFire(x, y, DIR_LEFT,  size, force, desc);
	if (dir & DIR_RIGHT) placeFire(x, y, DIR_RIGHT, size, force, desc);
}

int placeBomb(x, y, type, size, user)
	register u_short x, y, type;
	register u_char size, user;
{
	register Sprite *bomb;
	register u_short tileIndex = tile_index(x, y);

	/* Check if we can place a bomb */
	if ((user == 1) && (playerState.BombCur >= playerState.BombMax)) return 0;
	
	/* Create bomb; check if there are objects left */
	if (bomb = makeSprite(ST_BOMB, BOMB_SPRITE, 4, x, y)) {
		bomb->action = type | DIR_ALL | (BOMB_FRAMES * BOMB_TICK - 1);
		bomb->desc = size;
		bomb->data = tileIndex; /* Save tile index so we can mark the tile as walkable */
		bomb->word = user;
		gameLevel.tileMap[tileIndex].move |= 0x80; /* Mark tile as blocked */

		checkTrigger(x, y, 1);

		if (user == 1) {
			playerState.BombCur++;
			updateHud();
		}

		playSfx(SFX_FUSE);

		return 1;
	}
	else {
		return 0;
	}
}

void removeBomb(bomb)
	register Sprite *bomb;
{
	checkTrigger(bomb->x, bomb->y, 0);
	gameLevel.tileMap[bomb->data].move &= 0x7f; /* Unblock tile */
	bomb->type = ST_NONE;
}

void explodeBomb(bomb, dir)
	register Sprite *bomb;
	register u_short dir; /* Which direction to block*/
{
	register u_short actType = bomb->action & 0xf000;
	register u_short actDir  = bomb->action & 0x0f00;
	register Sprite *animObject;

	if (bomb->word == 1) {
		playerState.BombCur--;
		updateHud();
	}

	if (actType == ACT_HIDDEN) dir = DIR_NONE;

	removeBomb(bomb);

	if (canSpreadTo(bomb->data, DIR_ALL) != 0xFF) {
		/* Check which directions to blow the bomb */
		dir = (dir ^ DIR_ALL) & actDir;

		playSfx(SFX_BOOM);
		placeFires(bomb->x, bomb->y, DIR_ALL, bomb->desc, 1, bomb->word);
	}
	else {
		/* Bomb cannot explode - poof */
		animObject = makeSprite(ST_ANIM, 0xC4, 0, bomb->x, bomb->y);
		if (animObject != NULL) {
			animObject->action = ACT_ACTIVE | 11; /* Active for 12 frames */
			animObject->data = 0x0403; /* 4 frames, 3 steps/frame*/

			playSfx(SFX_CRACK);
		}
	}
}

void stepBomb(bomb)
	register Sprite *bomb;
{
	register u_short actType = bomb->action & 0xf000;
	register u_short actTime = bomb->action & 0x00ff;
	register int f;
	
	if (actType == ACT_HIDDEN) {

	}
	else if (actType == ACT_USER) {
		bomb->frame = (frameTick / BOMB_TICK) % BOMB_USER_FRAMES + BOMB_USER_START;
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

int attackTile(x, y, dir, attackType, data)
	u_short x, y, dir;
	u_char attackType, data;
{
	register u_short tileIndex = tile_index(x, y);
	register LevelTile *tile;
	register Sprite  *animObject;
	register u_char offset;
	
	attackSprite(x, y, dir, attackType, data);
	
	tile = &gameLevel.tileMap[tileIndex];

	if (tile->move & 0x10) {
		tile->move &= 0x6F; /* Clear BLOCK_PLAYER and DESTROYABLE bits */

		offset = (tile->top & 0x03) << 2;

		animObject = makeSprite(ST_ANIM, 0xC0 + offset, 0, x, y);
		if (animObject != NULL) {
			animObject->action = ACT_ACTIVE | 11; /* Active for 12 frames */
			animObject->data = 0x0403; /* 4 frames, 3 steps/frame*/
		}

		tile->top = 0x00;
		drawLevelTile(tileIndex);

		playSfx(SFX_CRACK);
		return P_TILE_CLEAR;
	}
	else {
		return 0;
	}
}

int checkWalkDir(x, y, dir)
	register u_short x, y, dir;
{
	register u_short tile = tile_index(x + dir_dx(dir), y + dir_dy(dir));
	register u_char value = gameLevel.tileMap[tile].move;
	return !(value &= 0xF0);
}

void walkFire(fire, checkDir)
	register Sprite *fire;
	register u_short checkDir;
{
	register int i, num;
	register u_short revDir = dir_rev(checkDir);

	num = rand();

	for (i = 0; i < 4; i++) {
		num = (num + 1) % 4;
		checkDir = DIR_LEFT << num;
		if ((revDir != checkDir) && checkWalkDir(fire->x, fire->y, checkDir)) {
			placeFire(fire->x + dir_dx(checkDir), fire->y + dir_dy(checkDir), checkDir, fire->data - 1, 1, fire->desc);
			return;
		}
	}
	placeFire(fire->x + dir_dx(revDir), fire->y + dir_dy(revDir), checkDir, fire->data - 1, 0, fire->desc);
}

void stepFire(fire)
	register Sprite *fire;
{	
	register u_short actDir   = fire->action & 0x0F00;
	register u_short actTime  = fire->action & 0x00FF;
	register u_short fireTime = fire->data;
	
	fire->action++;
	fire->frame = actTime;

	if (actTime == 0x05 && fire->data > 0) {
		if (fire->desc == 0xFF) {
			walkFire(fire, actDir);
		}
		else {
			placeFire(fire->x + dir_dx(actDir), fire->y + dir_dy(actDir), actDir, fire->data - 1, 0, fire->desc);
		}
	}
	else if (actTime == 0x08) {
		fire->type = ST_NONE;
	}
}

void placeProjectile(x, y, dir)
	register u_short x, y, dir;
{
	register Sprite *cannonBall;

	attackTile(x, y, dir, AT_PROJ, 0);

	cannonBall = makeSprite(ST_CNBALL, BALL_SPRITE, 0, x, y);
	if (cannonBall != NULL) {
		cannonBall->action = ACT_ACTIVE | dir | 0xFF;
		cannonBall->desc   = 0x21;   /* Move 1 pixel every 4 frames */
		cannonBall->data   = 0x0000; /* No animation */
	}
}

void placeProjectiles(x, y, dir)
	register u_short x, y, dir;
{
	if (dir & DIR_UP)    placeProjectile(x, y, DIR_UP);
	if (dir & DIR_DOWN)  placeProjectile(x, y, DIR_DOWN);
	if (dir & DIR_LEFT)  placeProjectile(x, y, DIR_LEFT);
	if (dir & DIR_RIGHT) placeProjectile(x, y, DIR_RIGHT);
}

void fireCannon(cannon, dir)
	register Sprite *cannon;
	register u_short dir;
{
	register u_short x = cannon->x + dir_dx(dir);
	register u_short y = cannon->y + dir_dy(dir);
	register Sprite *animObject;
	
	if (gameLevel.tileMap[tile_index(x, y)].move & BALL_MASK) return;

	playSfx(SFX_BOOM);

	placeProjectile(x, y, dir);

	animObject = makeSprite(ST_ANIM, 0xC4, 0, x, y);
	if (animObject != NULL) {
		animObject->action = ACT_ACTIVE | 11; /* Active for 12 frames */
		animObject->data = 0x0403; /* 4 frames, 3 steps/frame*/
	}
}

void stepCannon(cannon)
	register Sprite *cannon;
{
	register u_short actDir  = cannon->action & 0x0F00;
	register u_short actTime = cannon->action & 0x00FF;

	if (!cannon->data) return;

	if (!actTime) {
		fireCannon(cannon, actDir);
		cannon->action |= cannon->data;
	}
	else {
		cannon->action--;
	}
}

void stepProjectile(sprite)
	register Sprite *sprite;
{
	register u_short actDir  = sprite->action & 0x0F00;
	register short x = sprite->x + dir_dx(actDir);
	register short y = sprite->y + dir_dy(actDir);
	stepAnim(sprite);
	if (!((x | y) & 0x000f)) {
		if (attackTile(x, y, actDir, AT_PROJ, 0) || (gameLevel.tileMap[tile_index(x, y)].move & BALL_MASK)) {
			sprite->type = ST_NONE;
		}
	}

	/* Block at level bounds */
	if (x <= 0 | y <= 0 | x >= (368-16) | y >= (240-16)) sprite->type = ST_NONE;
}

void activateTorch(torch)
	register Sprite *torch;
{
	register u_short actDir  = torch->action & 0x0F00;
	register u_short actTime = torch->action & 0x00FF;

	if (actTime) {
		placeFire(torch->x + dir_dx(actDir), torch->y + dir_dy(actDir), actDir, 0xff, 1, 0xFF);
		torch->action--;
	}
}