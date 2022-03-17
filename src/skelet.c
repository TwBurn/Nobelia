#include <csd.h>
#include <sysio.h>
#include <ucm.h>
#include "game.h"
#include "audio.h"
#include "graphics.h"

u_char getTileValue(tile, source)
	register u_short tile, source;
{
	register u_short targetTile;
	register u_char move;
	register u_char result = 0;
	/* Result as nibbles: [[#MOVE][#DESTROY] ; Return 0x00 if there is a bomb nearby, so we don't go there */

	/* Left */
	targetTile = tile + dir_tile(DIR_LEFT);
	move = gameLevel.tileMap[targetTile].move;
	if (targetTile == source) { } /* Skip if source */
	else if ((move & 0x0F) == 0x00) {  } /* Skip, cannot move to tile */
	else if ((move & 0x10) != 0x00) { result |= 0x01; } /* Destroyable */
	else if ((move & 0x80) == 0x00) { result |= 0x10; } /* No bomb */
	else return 0x00;

	/* Right */
	targetTile = tile + dir_tile(DIR_RIGHT);
	move = gameLevel.tileMap[targetTile].move;
	if (targetTile == source) { } /* Skip if source */
	else if ((move & 0x0F) == 0x00) {  } /* Skip, cannot move to tile */
	else if ((move & 0x10) != 0x00) { result |= 0x01; } /* Destroyable */
	else if ((move & 0x80) == 0x00) { result |= 0x10; } /* No bomb */
	else return 0x00;
	
	/* Up */
	targetTile = tile + dir_tile(DIR_UP);
	move = gameLevel.tileMap[targetTile].move;
	if (targetTile == source) { } /* Skip if source */
	else if ((move & 0x0F) == 0x00) {  } /* Skip, cannot move to tile */
	else if ((move & 0x10) != 0x00) { result |= 0x01; } /* Destroyable */
	else if ((move & 0x80) == 0x00) { result |= 0x10; } /* No bomb */
	else return 0x00;

	/* Down */
	targetTile = tile + dir_tile(DIR_DOWN);
	move = gameLevel.tileMap[targetTile].move;
	if (targetTile == source) { } /* Skip if source */
	else if ((move & 0x0F) == 0x00) {  } /* Skip, cannot move to tile */
	else if ((move & 0x10) != 0x00) { result |= 0x01; } /* Destroyable */
	else if ((move & 0x80) == 0x00) { result |= 0x10; } /* No bomb */
	else return 0x00;

	return result;
}

#define BOMB_FILTER 0x80000000
u_int checkTileNeighbours(tile)
	register u_short tile;
{
	register u_short targetTile;
	register u_char tileValue;
	register u_int result = 0;

	/* Left */
	targetTile = tile + dir_tile(DIR_LEFT);
	if (canMoveTo(targetTile, DIR_LEFT)) {
		tileValue = getTileValue(targetTile, tile);
		result += tileValue << 0;
	}
	else if ((gameLevel.tileMap[targetTile].move & 0x8F) > 0x80) result |= BOMB_FILTER;

	/* Right */
	targetTile = tile + dir_tile(DIR_RIGHT);
	if (canMoveTo(targetTile, DIR_RIGHT)) {
		tileValue = getTileValue(targetTile, tile);
		result += tileValue << 8;
	}
	else if ((gameLevel.tileMap[targetTile].move & 0x8F) > 0x80) result |= BOMB_FILTER;
	
	/* Up */
	targetTile = tile + dir_tile(DIR_UP);
	if (canMoveTo(targetTile, DIR_UP)) {
		tileValue = getTileValue(targetTile, tile);
		result += tileValue << 16;
	}
	else if ((gameLevel.tileMap[targetTile].move & 0x8F) > 0x80) result |= BOMB_FILTER;

	/* Down */
	targetTile = tile + dir_tile(DIR_DOWN);
	if (canMoveTo(targetTile, DIR_DOWN)) {
		tileValue = getTileValue(targetTile, tile);
		result += tileValue << 24;
	}
	else if ((gameLevel.tileMap[targetTile].move & 0x8F) > 0x80) result |= BOMB_FILTER;

	return result;
}
Sprite* bone;
int boneX, boneY, boneDX, boneDY;

void destroyBone()
{
	Sprite *animObject;
	bone->action = ACT_HIDDEN | 80;
	playSfx(SFX_CRACK);

	animObject = makeSprite(ST_ANIM, 0xC4, 0, bone->x, bone->y);
		if (animObject != NULL) {
			animObject->action = ACT_ACTIVE | 11; /* Active for 12 frames */
			animObject->data = 0x0403; /* 4 frames, 3 steps/frame*/
		}
}

void stepBone(sprite)
	Sprite *sprite;
{
	register u_short actType = sprite->action & 0xf000;
	register u_short actDir  = sprite->action & 0x0f00;
	register u_short actTime = sprite->action & 0x00ff;
	Sprite *fire;

	if (actTime) sprite->action--;

	switch (actType) {
		case ACT_ACTIVE:
			boneX += boneDX;
			boneY += boneDY;
			sprite->x = boneX >> 16;
			sprite->y = boneY >> 16;
			
			if ((frameTick & 3) == 0) checkEnemyPlayer(sprite);
			else if ((frameTick & 3) == 2) {
				fire = findFire(sprite->x, sprite->y);
				if (fire != NULL && fire->desc != 2) {
					destroyBone();
					return;
				}
			}

			sprite->frame = (frameTick >> 2) & 0x07;

			if (sprite->x < 32 || sprite->x > 320 || sprite->y < 16 || sprite->y > 208) {
				destroyBone();
			}
		break;
	}
}

void createBone(sprite)
	Sprite *sprite;
{
	sprite->type = ST_BONE;
	sprite->action = ACT_HIDDEN;
	sprite->tile = 0xE0;
	sprite->frame = 0;
	sprite->desc = 0;
	sprite->data = 0;
	sprite->word = 0;

	bone = sprite;
}

void skeletMove(sprite, dir)
	Sprite *sprite;
	u_short dir;
{
	sprite->action = ACT_MOVE | dir | 0x0f;
	sprite->frame  = dir_frame(dir);
}

void skeletBomb(sprite, dir)
	Sprite *sprite;
	u_short dir;
{
	sprite->action = ACT_FIRE | dir | 0x08;
	sprite->frame  = dir_frame(dir) | 0x03;
	placeBomb(sprite->x + dir_dx(dir), sprite->y + dir_dy(dir), ACT_FIRE, 1, 2);
	sprite->data = 40;
}

#define MIN_THROW_RANGE 64 /* 4 tiles */
void skeletThrow(sprite, dx, dy)
	Sprite *sprite;
	short dx, dy;
{
	short xFact = dx > 0 ? dx : -dx;
	short yFact = dy > 0 ? dy : -dy;
	int factor = xFact + yFact;
	u_short dir;

	if (factor <= MIN_THROW_RANGE) return;

	if (xFact >= yFact)	{
		if (xFact < 0) dir = DIR_LEFT;
		else           dir = DIR_RIGHT;
	}
	else {
		if (yFact < 0) dir = DIR_UP;
		else           dir = DIR_DOWN;
	}

	sprite->action = ACT_FIRE | dir | 0x08;
	sprite->frame  = dir_frame(dir) | 0x03;
	sprite->data = 40;

	boneX = sprite->x << 16;
	boneY = sprite->y << 16;

	boneDX = (dx * 0x18000) / factor;
	boneDY = (dy * 0x18000) / factor;

	playSfx(SFX_ENHIT);
	bone->action = ACT_ACTIVE;
	stepBone(bone);
}

#define WF_MOVE 8
#define WF_AWAY 1
#define WF_BCLS 16
#define WF_BFAR 8
#define WF_WAIT 4
#define WF_BONE 4

u_short actionWeight[12];

void skeletAction(sprite)
	Sprite *sprite;
{
	register short x = sprite->x;
	register short y = sprite->y;
	register u_int value = checkTileNeighbours(tile_index(x, y));
	register u_short count = 0; 
	register u_short weight;
	register u_char moveAwayPrio;
	x -= player->x; y -= player->y;

	/* Check attack */
	if (value & BOMB_FILTER) {
		value &= 0x70707070;
		actionWeight[8] = 0;
		actionWeight[9] = 0;
		moveAwayPrio = WF_WAIT;
	}
	else if (sprite->data) {
		value &= 0x70707070;
		actionWeight[8] = 0;
		actionWeight[9] = WF_WAIT;
		moveAwayPrio = WF_WAIT;
	}
	else {
		actionWeight[8] = bone->action == ACT_HIDDEN ? WF_BONE : 0;
		actionWeight[9] = WF_WAIT;
		moveAwayPrio = WF_AWAY;
	}
	/* MOVE LEFT */
	weight = ((value >> 4) & 0x0000000F) * (x > 0 ? WF_MOVE : moveAwayPrio);
	count += weight; actionWeight[0] = count;

	/* MOVE RIGHT */
	weight = ((value >> 12) & 0x0000000F) * (x < 0 ? WF_MOVE : moveAwayPrio);
	count += weight; actionWeight[1] = count;

	/* MOVE UP */
	weight = ((value >> 20) & 0x0000000F) * (y > 0 ? WF_MOVE : moveAwayPrio);
	count += weight; actionWeight[2] = count;

	/* MOVE DOWN */
	weight = ((value >> 28) & 0x0000000F) * (y < 0 ? WF_MOVE : moveAwayPrio);
	count += weight; actionWeight[3] = count;
	
	/* ATTACK LEFT */
	weight = ((value >> 0) & 0x0000000F) * (x > 0 ? WF_BCLS : WF_BFAR);;
	count += weight; actionWeight[4] = count;

	/* MOVE RIGHT */
	weight = ((value >> 8) & 0x0000000F) * (x < 0 ? WF_BCLS : WF_BFAR);;
	count += weight; actionWeight[5] = count;

	/* MOVE UP */
	weight = ((value >> 16) & 0x0000000F) * (y > 0 ? WF_BCLS : WF_BFAR);;
	count += weight; actionWeight[6] = count;

	/* MOVE DOWN */
	weight = ((value >> 24) & 0x0000000F) * (y < 0 ? WF_BCLS : WF_BFAR);;
	count += weight; actionWeight[7] = count;

	/* THROW BONE */
	weight = actionWeight[8];
	count += weight; actionWeight[8] = count;

	/* WAIT */
	count += actionWeight[9];
/*
	printf("SA %d %02d %08X -  %d %d %d %d | %d %d %d %d | %d %d \n", sprite->data, count, value,
		actionWeight[0], actionWeight[1], actionWeight[2] ,actionWeight[3],
		actionWeight[4], actionWeight[5], actionWeight[6] ,actionWeight[7],
		actionWeight[8], actionWeight[9]
	);
*/
	if (count) {
		weight = rand() % count;

		if (weight < actionWeight[0]) { skeletMove(sprite, DIR_LEFT);  return; }
		if (weight < actionWeight[1]) { skeletMove(sprite, DIR_RIGHT); return; }
		if (weight < actionWeight[2]) { skeletMove(sprite, DIR_UP);    return; }
		if (weight < actionWeight[3]) { skeletMove(sprite, DIR_DOWN);  return; }
		if (weight < actionWeight[4]) { skeletBomb(sprite, DIR_LEFT);  return; }
		if (weight < actionWeight[5]) { skeletBomb(sprite, DIR_RIGHT); return; }
		if (weight < actionWeight[6]) { skeletBomb(sprite, DIR_UP);    return; }
		if (weight < actionWeight[7]) { skeletBomb(sprite, DIR_DOWN);  return; }
		if (weight < actionWeight[8]) { skeletThrow(sprite, -x, -y);   return; }
	}
	sprite->action |= (rand() & 0x0F) + 8; /* Random Delay between moves */
}

void skeletDown(sprite)
	Sprite *sprite;
{
	register u_short actTime = sprite->action & 0x00ff;

	if (sprite->desc) {
		if (actTime == 0) {

			if (sprite->desc == 2) {
				sprite->action = ACT_ACTIVE | DIR_DOWN | 0x1f;
				sprite->x = 3 << 4;
				sprite->y = 2 << 4;
				sprite->frame = 0;
			}
			else {
				sprite->action = ACT_ACTIVE | DIR_UP | 0x1f;
				sprite->x = 19 << 4;
				sprite->y = 12 << 4;
				sprite->frame = 0;
			}
			sprite->data = 80;
			
			return;
		}
		else if (actTime < 80) {
			switch (actTime & 0x03) {
				case 1: sprite->x = sprite->data - 1; break;
				case 3: sprite->x = sprite->data + 1; break;
				default: sprite->x = sprite->data; break;
			}
		}

		sprite->action--;
	}
}

void stepSkeletBoss(sprite)
	Sprite *sprite;
{
	register u_short actType = sprite->action & 0xf000;
	register u_short actDir  = sprite->action & 0x0f00;
	register u_short actTime = sprite->action & 0x00ff;

	switch (actType) {
		case ACT_DEAD: return;
		case ACT_HIT:
			skeletDown(sprite);
		return;
		case ACT_FIRE:
		case ACT_ACTIVE:
			if (sprite->data) sprite->data--;
			if (actTime == 0) {
				skeletAction(sprite);
				return;
			}
			else {
				actTime--;
			}
		break;
		case ACT_MOVE:
			if (sprite->data) sprite->data--;
			sprite->x += pix_dx(actDir);
			sprite->y += pix_dy(actDir);

			if (actTime == 0) {
				actType = ACT_ACTIVE;
				actTime = (rand() & 0x08); /* Random Delay between moves */
			}
			else {
				actTime--;
			}

			walkAnim(sprite);
		break;
	}

	sprite->action = actType | actDir | actTime;
	checkEnemyPlayer(sprite);
}



void stepSkeletCastle(sprite)
	Sprite *sprite;
{
	register u_short actType = sprite->action & 0xf000;
	register u_short actDir  = sprite->action & 0x0f00;
	register u_short actTime = sprite->action & 0x00ff;

	switch (actType) {
		case ACT_IDLE:
			/* Check if bridge is down */
			if (gameLevel.tileMap[tile_pos(11, 5)].top) {
				sprite->action = ACT_MOVE | DIR_UP | 0x1f;
			}
		break;
		case ACT_MOVE:
			if (frameTick & 1) return;
			sprite->y--;
			sprite->frame = 0x0d + ((sprite->y >> 2) & 1);
			if (actTime == 0) {
				updateLevelTile(tile_pos(11, 1), 0x56, 0xCF, 0xC4);
				sprite->type = ST_NONE;
			}
			else {
				sprite->action--;
			}
		break;
	}
}


void stepSkelet(sprite)
	Sprite *sprite;
{
	if (sprite->desc == 0xFF) {
		stepSkeletCastle(sprite);
	}
	else {
		stepSkeletBoss(sprite);
	}
}

void hitSkelet(sprite)
	Sprite *sprite;
{
	register u_short actType = sprite->action & 0xf000;
	if (actType == ACT_HIT || actType == ACT_DEAD) return;
	
	sprite->desc--;

	if (sprite->desc) {
		sprite->action = ACT_HIT;
		sprite->data = sprite->x;
		sprite->frame = 0x8F - sprite->tile;
		playSfx(SFX_ENHIT);
		sprite->action |= 240;
	}
	else {
		killSprite(sprite);

		updateLevelTile(tile_pos(17, 1), 0x8f, 0xE0, 0x00);
		updateLevelTile(tile_pos(18, 1), 0x00, 0x0f, 0xc8);
		updateLevelTile(tile_pos(19, 1), 0x8d, 0xE0, 0x00);

		updateLevelTile(tile_pos(4, 13), 0x0A, 0x0F, 0xc4);
		updateLevelTile(tile_pos(4, 14), 0x0A, 0xE0, 0xc4);
		playSfx(SFX_POWER);
	}
}
