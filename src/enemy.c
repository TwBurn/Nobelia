#include <csd.h>
#include <sysio.h>
#include <ucm.h>
#include "graphics.h"
#include "game.h"
#include "audio.h"

char canMoveSprite(sprite, dir)
	Sprite *sprite;
	u_short dir;
{
	register u_short targetTile = tile_index(sprite->x, sprite->y) + dir_tile(dir);
	register char value = gameLevel.tileMap[targetTile].move;
	register char check = dir >> 8;
	return (value & 0x90) ? 0 : value & check;
}

void checkEnemyPlayer(sprite)
	Sprite *sprite;
{
	register short dx = player->x - sprite->x;
	register short dy = player->y - sprite->y;
	dx = dx > 0 ? dx : -dx;
	dy = dy > 0 ? dy : -dy;

	if (dx < 14 && dy < 14) {
		killSprite(player);
	}
}

/*

Ghost is implemented in a script

void stepGhost(sprite)
	Sprite *sprite;
{

}
*/

void stepBlob(sprite)
	Sprite *sprite;
{
	register u_short actType = sprite->action & 0xf000;
	register u_short actDir  = sprite->action & 0x0f00;
	register u_short actTime = sprite->action & 0x00ff;
	register u_short checkDir;
	register int random;
	
	switch(actType) {
		case ACT_DEAD: return;
		case ACT_FIRE: 
			if (actTime == 0) {
				sprite->action = sprite->data;
			}
			else {
				sprite->action--;
			}
			sprite->frame = 0x0f;
		return;
		case ACT_MOVE:
			sprite->x += pix_dx(actDir);
			sprite->y += pix_dy(actDir);
			if (actTime == 0) {
				actType = ACT_IDLE;
				actTime = 13; /* Delay between moves */
			}
			else {
				actTime--;
			}
		break;
		case ACT_IDLE:
			if (actTime == 0) {
				actType = ACT_ACTIVE;
			}
			else {
				actTime--;
			}
		break;
	 	case ACT_ACTIVE:
			random = rand() & 0x03;
			/* Determine next direction */
			switch(random) {
				case 0: random = 0x3210; break;
				case 1: random = 0x3120; break;
				case 2: random = 0x3012; break;
				case 3: random = 0x3021; break;
			}
			while(random > 0) {
				switch (random & 0x000f) {
					case 0: checkDir = actDir; break;
					case 1: checkDir = dir_prev(actDir); break;
					case 2: checkDir = dir_next(actDir); break;
					case 3: checkDir = dir_rev(actDir); break;
				}
				if (canMoveSprite(sprite, checkDir)) {
					actDir = checkDir;
					break;
				}
				random >>= 4;
			}
			
			actType = ACT_MOVE;
			actTime = 15; /* Move 16 pixels */
		break;
	}

	switch(actDir) {
		case DIR_LEFT:  sprite->frame = 0x05 + ((sprite->x >> 2) & 1); break;
		case DIR_RIGHT: sprite->frame = 0x09 + ((sprite->x >> 2) & 1); break;
		case DIR_UP:    sprite->frame = 0x0d + ((sprite->y >> 2) & 1); break;
		case DIR_DOWN:  sprite->frame = 0x01 + ((sprite->y >> 2) & 1); break;
	}

	sprite->action = actType | actDir | actTime;
	checkEnemyPlayer(sprite);
}

#define SPIDER_HOLE_COUNT 13
#define SPIDER_EXIT_DISTANCE 50
u_short spiderHoles[SPIDER_HOLE_COUNT] = {
	tile_pos(8, 2), tile_pos(18, 2), tile_pos(4, 3), tile_pos(9, 4), tile_pos(14, 4),
	tile_pos(3, 5), tile_pos(12, 6), tile_pos(6, 7), tile_pos(16, 8), tile_pos(18, 9),
	tile_pos(7, 10), tile_pos(10, 11), tile_pos(15, 11)
};


u_short checkNeighbourSpider(x, y)
	register short x, y;
{
	register u_short tile = tile_index(x, y);
	register u_short targetTile;
	register u_short result = 0;
	
	x -= player->x; y -= player->y;
	
	/* Left */
	targetTile = tile + dir_tile(DIR_LEFT);
	if (canMoveTo(targetTile, DIR_LEFT))              result += 0x0001;
	if (gameLevel.tileMap[targetTile].middle == 0xE7 && gameLevel.tileMap[targetTile].top == 0x00) result += 0x0002;
	if (gameLevel.tileMap[targetTile].move & 0x10)    result += 0x0004;
	if (x > 0) result += 0x0008;

	/* Right */
	targetTile = tile + dir_tile(DIR_RIGHT);
	if (canMoveTo(targetTile, DIR_RIGHT))             result += 0x0010;
	if (gameLevel.tileMap[targetTile].middle == 0xE7 && gameLevel.tileMap[targetTile].top == 0x00) result += 0x0020;
	if (gameLevel.tileMap[targetTile].move & 0x10)    result += 0x0040;
	if (x < 0) result += 0x0080;
	
	/* Up */
	targetTile = tile + dir_tile(DIR_UP);
	if (canMoveTo(targetTile, DIR_UP))                result += 0x0100;
	if (gameLevel.tileMap[targetTile].middle == 0xE7 && gameLevel.tileMap[targetTile].top == 0x00) result += 0x0200;
	if (gameLevel.tileMap[targetTile].move & 0x10)    result += 0x0400;
	if (y > 0) result += 0x0800;

	/* Down */
	targetTile = tile + dir_tile(DIR_DOWN);
	if (canMoveTo(targetTile, DIR_DOWN))              result += 0x1000;
	if (gameLevel.tileMap[targetTile].middle == 0xE7 && gameLevel.tileMap[targetTile].top == 0x00) result += 0x2000;
	if (gameLevel.tileMap[targetTile].move & 0x10)    result += 0x4000;
	if (y < 0) result += 0x8000;

	return result;
}


int checkOpenHoles()
{
	register int i = 0, count = 0;
	for (i = 0; i < SPIDER_HOLE_COUNT; i++) {
		if (gameLevel.tileMap[spiderHoles[i]].top == 0) count++;
	}
	return count;
}

u_short findOpenTile(tile)
	register u_short tile;
{
	register u_short checkTile;

	checkTile = tile;
	if (canMoveTo(checkTile, DIR_ALL)) return checkTile;

	checkTile = tile + dir_tile(DIR_UP);
	if (canMoveTo(checkTile, DIR_ALL)) return checkTile;
	checkTile = tile + dir_tile(DIR_DOWN);
	if (canMoveTo(checkTile, DIR_ALL)) return checkTile;
	checkTile = tile + dir_tile(DIR_LEFT);
	if (canMoveTo(checkTile, DIR_ALL)) return checkTile;
	checkTile = tile + dir_tile(DIR_RIGHT);
	if (canMoveTo(checkTile, DIR_ALL)) return checkTile;

	return tile_pos(11, 7);
}

void killSpider(sprite)
	Sprite *sprite;
{
	register u_short actType = sprite->action & 0xf000;
	register u_short tile = tile_index(sprite->x, sprite->y);
	register u_short i;
	
	if ((actType == ACT_HIDDEN) || (actType == ACT_DEAD)) return;

	sprite->desc--;

	if (sprite->desc > 0) {
		/* Hide Spider, reset hole counter */
		sprite->action = ACT_HIDDEN | 64;
		sprite->data = 0;
		playSfx(SFX_ENHIT);
		return;
	}
	
	for (i = 0; i < SPRITE_COUNT; i++) {
		if (gameLevel.object[i].type == ST_CHEST) {
			tile = findOpenTile(tile);

			gameLevel.object[i].tile = 0xA0;
			gameLevel.object[i].action = ACT_IDLE;
			gameLevel.object[i].x = tile_x(tile);
			gameLevel.object[i].y = tile_y(tile);

			/* Only block tile when there is a chest */
			gameLevel.tileMap[tile].move |= 0x80;
		}
	}

	killSprite(sprite);
}

void stepSpiderInHole(sprite)
	Sprite *sprite;
{
	register short dx;
	register short dy;
	register u_int holeNumber;
	register u_short holeTile;

	if (sprite->data == 0) {
		if (checkOpenHoles() >= (12 - 3 * sprite->desc)) {
			sprite->data = 1;
		}
		else {
			sprite->action += 7;
			return;
		}
	}
	holeNumber = frameTick % SPIDER_HOLE_COUNT;
	holeTile = spiderHoles[holeNumber];

	sprite->x = tile_x(holeTile);
	sprite->y = tile_y(holeTile);

	dx = player->x - sprite->x; dx = dx < 0 ? -dx : dx;
	dy = player->y - sprite->y; dy = dy < 0 ? -dy : dy;

	if (gameLevel.tileMap[holeTile].top == 0 && (dx + dy) > SPIDER_EXIT_DISTANCE) {
		sprite->action = ACT_IDLE | 0x1f;
		sprite->frame = 0;
	}
	else {
		sprite->action += 7;
	}
}

#define WEIGHT_NEAR 4
#define WEIGHT_HOLE 2
#define WEIGHT_MOVE 1

u_short findSpiderMoveDirection(value)
	register u_short value;
{
	register u_short count = 0;
	register u_short valR, valL, valU, valD;
	register u_short filter;
	register u_int random;

/* Assign weight to direction types, closer, enter hole, further away */

	/* Check LEFT */
	filter = value & 0x000B;
	if      (filter == 0x0009) count += WEIGHT_NEAR;
	else if (filter == 0x0002) count += WEIGHT_HOLE;
	else if (filter == 0x0001) count += WEIGHT_MOVE;
	valL = count;

	/* Check RIGHT */
	filter = value & 0x00B0;
	if      (filter == 0x0090) count += WEIGHT_NEAR;
	else if (filter == 0x0020) count += WEIGHT_HOLE;
	else if (filter == 0x0010) count += WEIGHT_MOVE;
	valR = count;

	/* Check UP */
	filter = value & 0x0B00;
	if      (filter == 0x0900) count += WEIGHT_NEAR;
	else if (filter == 0x0200) count += WEIGHT_HOLE;
	else if (filter == 0x0100) count += WEIGHT_MOVE;
	valU = count;

	/* Check DOWN */
	filter = value & 0xB000;
	if      (filter == 0x9000) count += WEIGHT_NEAR;
	else if (filter == 0x2000) count += WEIGHT_HOLE;
	else if (filter == 0x1000) count += WEIGHT_MOVE;
	valD = count;

	random = (rand() % count) + 1;

	if (random <= valL) return DIR_LEFT;
	if (random <= valR) return DIR_RIGHT;
	if (random <= valU) return DIR_UP;
	if (random <= valD) return DIR_DOWN;
}

void stepSpider(sprite)
	Sprite *sprite;
{
	register u_short actType = sprite->action & 0xf000;
	register u_short actDir  = sprite->action & 0x0f00;
	register u_short actTime = sprite->action & 0x00ff;
	register u_short value;

	switch (actType) {
		case ACT_DEAD: return;
		case ACT_HIDDEN:
			if (actTime == 0) {
				stepSpiderInHole(sprite);
			}
			else {
				sprite->action--;
			}
		return;
		case ACT_MOVE:
			if ((actTime & 1) == 0) {
				sprite->x += pix_dx(actDir);
				sprite->y += pix_dy(actDir);
			}
			if (actTime == 0) {
				if (gameLevel.tileMap[tile_index(sprite->x, sprite->y)].middle == 0xE7) {
					actType = ACT_HIDDEN;
					actTime = (rand() % 64) + 32;
				}
				else {
					actType = ACT_IDLE;
				}
			}
			else {
				actTime--;
			}
		break;
	}

	if (actType == ACT_IDLE) {
		if (actTime == 0) {
			value = checkNeighbourSpider(sprite->x, sprite->y);
			
			actType = ACT_MOVE;
			actDir = findSpiderMoveDirection(value);
			actTime = 0x1F;
		}
		else {
			actTime--;
		}
	}

	switch(actDir) {
		case DIR_LEFT:  sprite->frame = 0x05 + ((sprite->x >> 2) & 1); break;
		case DIR_RIGHT: sprite->frame = 0x09 + ((sprite->x >> 2) & 1); break;
		case DIR_UP:    sprite->frame = 0x0d + ((sprite->y >> 2) & 1); break;
		case DIR_DOWN:  sprite->frame = 0x01 + ((sprite->y >> 2) & 1); break;
	}

	sprite->action = actType | actDir | actTime;
	checkEnemyPlayer(sprite);
}

void stepBat(sprite)
	Sprite *sprite;
{
	register u_short actType = sprite->action & 0xf000;
	register u_short actDir  = sprite->action & 0x0f00;
	register u_short actTime = sprite->action & 0x00ff;
	register short dx;
	register short dy;
	register u_int random;
	
	

	switch (actType) {
		case ACT_DEAD: return;
		case ACT_IDLE:
			actDir = player->action & 0x0F00;

			dx = player->x - sprite->x;
			dy = player->y - sprite->y;

			/* Determine target by circling around player */
			random = frameTick >> 2;
			actTime = random >> 6 & 0x03;
			random &= 63;

			switch(actTime) {
				case 0: dx += 32 - random; dy -= 32; break;
				case 1: dx -= 32; dy += random - 32; break;
				case 2: dx += random - 32; dy += 32; break;
				case 3: dx += 32; dy += 32 - random; break;
			}
			
			random = rand();
			
			if ((random & 1) == 0) {
				if (dx < 0) actDir = DIR_LEFT;
				else actDir = DIR_RIGHT;
			}
			else {
				if (dy < 0) actDir = DIR_UP;
				else actDir = DIR_DOWN;
			}
			sprite->action = ACT_MOVE | actDir | 9;
			break;
		case ACT_MOVE:
			if ((actTime & 3) == 0) {
				sprite->x += pix_dx(actDir);
				sprite->y += pix_dy(actDir);
			}
			
			if (!actTime) {
				sprite->action = ACT_IDLE | actDir; /* Free to move for the next 4 frames */
			}
			else {
				sprite->action--;
			}
			break;
	}

	switch(actDir) {
		case DIR_LEFT:  sprite->frame = 0x05 + ((sprite->x >> 2) & 1); break;
		case DIR_RIGHT: sprite->frame = 0x09 + ((sprite->x >> 2) & 1); break;
		case DIR_UP:    sprite->frame = 0x0d + ((sprite->y >> 2) & 1); break;
		case DIR_DOWN:  sprite->frame = 0x01 + ((sprite->y >> 2) & 1); break;
	}

	checkEnemyPlayer(sprite);
}

void enemyHit(sprite, attackType, data)
	Sprite *sprite;
	register u_char attackType;
{
	register u_short actType = sprite->action & 0xf000;
	switch(sprite->tile) {
		case ET_SPIDER:
			killSpider(sprite);
		break;
		case ET_BAT:
			killSprite(sprite);
		break;
		case ET_BLOB:
			if (actType == ACT_FIRE || actType == ACT_DEAD) return;
			
			
			/* Hit by fire -> Life -1 */
			/* Hit by projectile -> Dead */
			if (attackType == AT_FIRE) sprite->desc--;
			if (attackType == AT_PROJ) sprite->desc = 0;
			
			if (sprite->desc == 0) { 
				killSprite(sprite);
				return;
			}
			
			
			sprite->data = sprite->action;
			sprite->action = ACT_FIRE | 0x83;
			playSfx(SFX_ENHIT);
		break;
		case ET_SKELET:
			if (data != 2) hitSkelet(sprite);
		break;
	}
}

void stepEnemy(sprite)
	Sprite *sprite;
{
	switch(sprite->tile) {
		case ET_SKELET:	stepSkelet(sprite);	break;
		/*case ET_GHOST:	stepGhost(sprite);	break;*/
		case ET_BLOB:	stepBlob(sprite);	break;
		case ET_SPIDER:	stepSpider(sprite);	break;
		case ET_BAT:	stepBat(sprite);	break;
	}
}
