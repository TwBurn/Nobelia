#include <csd.h>
#include <sysio.h>
#include <ucm.h>
#include "graphics.h"
#include "audio.h"
#include "game.h"

#define SCRIPT_INTRO         1000
#define SCRIPT_INIT_TEMPLE   1001
#define SCRIPT_REMOVE_CANNON 1004
#define SCRIPT_CAVE_HALLWAY  1005
#define SCRIPT_START_BURNERS 1007
#define SCRIPT_WATER_BOSS    1012
#define SCRIPT_ROCK_ENTRANCE 1013
#define SCRIPT_ROCK_GRASS    1014
#define SCRIPT_ROCK_BOSS     1016
#define SCRIPT_OPEN_EXIT     1017
#define SCRIPT_GRASS_TUNNEL  1018
#define SCRIPT_GRASS_WOODS   1019
#define SCRIPT_GRASS_BOSS    1021
#define SCRIPT_MAIN_BONUS    1022
#define SCRIPT_ONOFF_BURNERS 1024
#define SCRIPT_TEMPLE_MAZE   1027
#define SCRIPT_FINAL_CASTLE  1028
#define SCRIPT_FINAL_BOSS    1029

void updateLevelTile(index, top, move, anim)
	register u_short index;
	register u_char top, move, anim;
{
	register Sprite *animObject;

	gameLevel.tileMap[index].top = top;
	gameLevel.tileMap[index].move = move;
	drawLevelTile(index);

	if (anim) {
		animObject = makeSprite(ST_ANIM, anim, 0, tile_x(index), tile_y(index));
		if (animObject != NULL) {
			animObject->action = ACT_ACTIVE | 11; /* Active for 12 frames */
			animObject->data = 0x0403; /* 4 frames, 3 steps/frame*/
		}
	}
}

void stepIntro(sprite) 
	register Sprite* sprite;
{
	register short dx = sprite->x - player->x;
	register short dy = sprite->y - player->y;

	dx = 2 * (dx >= 0 ? dx : -dx);
	dy = 2 * (dy >= 0 ? dy : -dy);

	if ((dx + dy) < sprite->data) {
		sprite->data = dx + dy;
	}
	else {
		sprite->data--;
	}

	if (sprite->data == 0) {
		switch(sprite->desc) {
			case 0:	sprite->x = 192; sprite->y = 176; break;
			case 1: sprite->x =  80; sprite->y = 160; break;
			case 2: sprite->x =  48; sprite->y =  32;
				playSfx(SFX_POWER);
				updateLevelTile(tile_pos(6, 5), 0xDB, 0x0F, 0xC4);
				break;
			case 3:
				placeBomb(96, 80, ACT_ACTIVE, 1, 0);
				sprite->type = ST_NONE;
				break;
		}
		sprite->desc++;
		sprite->data = 500;
	}
}

Sprite* wbGhost  = NULL;
Sprite* wbChest  = NULL;
Sprite* wbTarget = NULL;
Sprite* wbSprite[4];
short wbX, wbY;
short wbCount;
/*
short wbDX[] = {-72,-71,-67,-60,-51,-40,-28,-14,0,14,28,40,51,60,67,71,72,71,67,60,51,40,28,14,0,-14,-28,-40,-51,-60,-67,-71};
short wbDY[] = {0,14,28,40,51,60,67,71,72,71,67,60,51,40,28,14,0,-14,-28,-40,-51,-60,-67,-71,-72,-71,-67,-60,-51,-40,-28,-14};
*/

short wbSine[] = {0,19,37,53,68,80,89,94,96,94,89,80,68,53,37,19,0,-19,-37,-53,-68,-80,-89,-94,-96,-94,-89,-80,-68,-53,-37,-19};
u_char wbIcf[] = {23, 33, 43, 53, 63};

int wbDX(index)
	int index;
{
	return wbSine[(index + 24) & 0x1F] * 8 / (wbCount + 8);
}

int wbDY(index)
	int index;
{
	return wbSine[index] * 8 / (wbCount + 8);
}


int wbDistanceTo(sprite, dx, dy)
	Sprite* sprite;
	register short dx, dy;
{
	register int x = (dx + wbX - (sprite->x << 6)) >> 4;
	register int y = (dy + wbY - (sprite->y << 6)) >> 4;
	return x * x + y * y;
}

int wbRange(sprite)
	Sprite* sprite;
{  /* Return max(dx, dy) */
	register short dx = sprite->x - wbGhost->x;
	register short dy = sprite->y - wbGhost->y;
	dx = dx >= 0 ? dx : -dx;
	dy = dy >= 0 ? dy : -dy;
	return dx <= dy ? dy : dx;
}

int targetWaterBoss()
{
	register int i, n, min, range;
	
	range = wbRange(player);

	if (range < 16) {
		killSprite(player);
	}

	min = range + 8 * (wbCount + 1); /* Give some priority to pedestals */
	wbTarget = player;
	
	n = 0;
	for (i = 0; i < 4; i++) {
		if (wbSprite[i]->action == ACT_ACTIVE) {
			range = wbRange(wbSprite[i]);
			if (wbRange(wbSprite[i]) <= 24) {
				wbSprite[i]->action = ACT_IDLE;
			}
			else {
				n++;
				if (range < min) {
					wbTarget = wbSprite[i];
					min = range;
				}
			}
		}
	}
	return n;
}

void moveWaterBoss()
{
	register int lPrev, lNext;
	register u_short dCur, dPrev, dNext;
	


	dCur = wbGhost->desc;
	dPrev = (dCur + 0x1f) & 0x1f;
	dNext = (dCur + 0x01) & 0x1f;
	
	lPrev = wbDistanceTo(wbTarget, wbDX(dPrev), wbDY(dPrev));
	lNext = wbDistanceTo(wbTarget, wbDX(dNext), wbDY(dNext));

	if (lNext <= lPrev) {
		wbGhost->desc = dNext;
	}
	else {
		wbGhost->desc = dPrev;
	}
}

void waterBoss(sprite)
	register Sprite* sprite;
{
	int i, n;
	switch(sprite->desc) {
		case 0: /* Init Level */
			n = 0;
			for (i = 0; i < SPRITE_COUNT; i++) {
				switch (gameLevel.object[i].type) {
					case ST_PLAYER:  wbTarget      = &gameLevel.object[i]; break; /* Find Player */
					case ST_ENEMY:   wbGhost       = &gameLevel.object[i]; break; /* Find Ghost */
					case ST_CHEST:   wbChest       = &gameLevel.object[i]; break; /* Find Cloak */
					case ST_PEDESTL: wbSprite[n++] = &gameLevel.object[i]; break; /* Find Pedestals */
				}
			}
			wbX = wbGhost->x << 6;
			wbY = wbGhost->y << 6;

			sprite->desc++;

			if (wbChest == NULL) {
				/* Chest has already been opened -> Save loaded, deactivate script */
				sprite->type = ST_NONE;
			}
		break;
		case 1:
			if (wbChest->type != ST_CHEST) {
				wbGhost->action = ACT_IDLE;
				for (i = 0; i < 4; i++) { /* Deactivate Pedestals */
					wbSprite[i]->action = ACT_IDLE;
				}
				setIcf(ICF_MAX, wbIcf[0]);
				/* Remove Bridge */
				updateLevelTile(tile_pos(10, 11), 0x00, 0xC0, 0xC4);
				updateLevelTile(tile_pos(10, 12), 0x00, 0xC0, 0xC4);
				updateLevelTile(tile_pos(10, 13), 0x00, 0xC0, 0xC4);
				sprite->desc++;
			}
		break;
		case 2:
			/* Move and flicker ghost */
			if ((frameTick % 10) == 0) {
				moveWaterBoss();
			}
			if ((frameTick & 0x01) == 0) {
				wbGhost->frame = (frameTick & 16) >> 4;

				wbX += wbDX(wbGhost->desc);
				wbY += wbDY(wbGhost->desc);

				wbGhost->x = wbX >> 6;
				wbGhost->y = wbY >> 6;

				wbCount = targetWaterBoss();
				setIcf(ICF_MAX, wbIcf[wbCount]);
				if (wbCount == 4) {
					killSprite(wbGhost);
					updateLevelTile(tile_pos(10, 11), 0x0A, 0x0F, 0xC4);
					updateLevelTile(tile_pos(10, 12), 0x0A, 0x0F, 0xC4);
					updateLevelTile(tile_pos(10, 13), 0x0A, 0x0F, 0xC4);
					
					sprite->type = ST_NONE;
				}
			}
		break;
	}
}

void rockBoss(count)
	register int count;
{
	if (count == 2) {
		updateLevelTile(tile_pos(6, 13), 0x00, 0x0f, 0xc8);
		updateLevelTile(tile_pos(7, 13), 0x00, 0x0f, 0xc8);
		updateLevelTile(tile_pos(8, 13), 0x00, 0x0f, 0xc8);
		removeTriggers();
	}
}

void rockEntrance(trigger, count)
	register Sprite* trigger;
	register int count;
{
	register u_char top = gameLevel.tileMap[tile_pos(16, 7)].top;

	if (top == 0 && (count < trigger->data)) {
		updateLevelTile(tile_pos(16, 7), 0xc9, 0xcf, 0xc4);
		updateLevelTile(tile_pos(16, 8), 0xc8, 0xcf, 0xc4);
	}
	else if (top != 0 && (count >= trigger->data)) {
		updateLevelTile(tile_pos(16, 7), 0x00, 0x0f, 0xc4);
		updateLevelTile(tile_pos(16, 8), 0x00, 0x0f, 0xc4);
	}
}

void rockGrass(count)
	register int count;
{
	if (count == 2) {
		updateLevelTile(tile_pos(10, 1), 0x00, 0x0f, 0xc8);
		updateLevelTile(tile_pos(11, 1), 0x00, 0x0f, 0xc8);
		updateLevelTile(tile_pos(12, 1), 0x00, 0x0f, 0xc8);
		removeTriggers();
	}
}

void caveHallway(count)
	register int count;
{
	if (count == 2) {
		startBurners();
		removeTriggers();
	}
}

void grassTunnel(sprite, count)
	register Sprite* sprite;
	register int count;
{
	register int i;
	/* Check if state is unchanged */
	if (sprite->data == count) { return; } 
	sprite->data = count;

	if (count == 1) {
		/* Bomb on trigger */
		updateLevelTile(tile_pos(10, 2), 0x00, 0x0f, 0xc4);
		updateLevelTile(tile_pos( 3, 7), 0xe3, 0x0f, 0xc4);

	}
	else {
		/* No bomb */
		updateLevelTile(tile_pos(10, 2), 0xe3, 0x0f, 0xc4);
		updateLevelTile(tile_pos( 3, 7), 0x00, 0xef, 0xc4);
	}

	/* Toggle exit at tile_pos(10, 2) */
	for (i = 0; i < EXIT_COUNT; i++) {
		if (gameLevel.exit[i].tile == tile_pos(10, 2)) {
			gameLevel.exit[i].type ^= 0x80;
		}
	}
}

void grassWoods()
{
	openExit(tile_pos(15, 3));
	updateLevelTile(tile_pos(15, 3), 0x00, 0x0f, 0xc4);
	updateLevelTile(tile_pos(15, 9), 0x00, 0x04, 0xc4);
}

void grassBoss(sprite, count)
	register Sprite* sprite;
	register int count;
{
	/* Check if state is unchanged */
	if (sprite->data == count) { return; } 
	sprite->data = count;

	if (count == 1) {
		/* Bomb on trigger */
		updateLevelTile(tile_pos(11, 2), 0x00, 0x0f, 0xc4);
		gameLevel.exit[0].type &= 0x7f;

	}
	else {
		/* No bomb */
		updateLevelTile(tile_pos(11, 2), 0xe0, 0x0f, 0xc4);
		gameLevel.exit[0].type |= 0x80;
	}
}

void mainBonus(count)
	register int count;
{
	if (count == 2) {
		updateLevelTile(tile_pos(10, 3), 0x00, 0x0f, 0xc8);
		updateLevelTile(tile_pos(11, 3), 0x00, 0x0f, 0xc8);
		updateLevelTile(tile_pos(12, 3), 0x00, 0x0f, 0xc8);
		removeTriggers();
	}
}

u_char templeDirections[4];
u_char templeExit = 0;
LevelExit templeExitWrong = {
	0,  /* tile; */
	26, /* next; */
	0,  /* direction; */
	2,  /* type; */
	8,  /* spawnDir; */
	tile_pos(14, 7) /* spawnTile; */
};

LevelExit templeExitNext = {
	0,  /* tile; */
	27, /* next; */
	0,  /* direction; */
	2,  /* type; */
	8,  /* spawnDir; */
	tile_pos(11, 7) /* spawnTile; */
};

LevelExit templeExitFinal = {
	0,  /* tile; */
	28, /* next; */
	0,  /* direction; */
	2,  /* type; */
	8,  /* spawnDir; */
	tile_pos(7, 12) /* spawnTile; */
};

void initTempleMaze(sprite)
	Sprite* sprite;
{
	u_int i, value;
	u_char dir;
	if (playerState.BombMax) {
		/* Randomize Directions for Temple/Maze when player enters this screen after the bomb has been picked up */
		if (templeValue == 0) {
			templeValue = (rand() & 0x00FF) | 0x0100;
		}

		value = templeValue;
		
		for (i = 0; i < 4; i++) {
			dir = value & 3;
			templeDirections[i] = (1 << dir);
			updateTempleStatue(i, dir);
			value = value >> 2;
		}
		updateIntroLevel();
		sprite->type = ST_NONE;
	}
}

void stepTempleMaze(sprite)
	register Sprite* sprite;
{
	register u_short exitDir = DIR_NONE;

	if ((player->action & 0xf000) == ACT_EXIT) return;

	if (player->x < 80) {
		exitDir = DIR_LEFT;
	}
	else if (player->x > 272) {
		exitDir = DIR_RIGHT;
	}
	else if (player->y < 64) {
		exitDir = DIR_UP;
	}
	else if (player->y > 176) {
		exitDir = DIR_DOWN;
	}

	if (exitDir > 0) {
		if ((exitDir >> 8) != templeDirections[templeExit]) {
			/* Wrong Direction */
			templeExit = 0;
			exitLevel(&templeExitWrong, exitDir);
			playSfx(SFX_MZFAIL);
		}
		else if (templeExit < 3) {
			/* Next Direction */
			templeExit += 1;
			exitLevel(&templeExitNext, exitDir);
			playSfx(SFX_POWER);
		}
		else {
			/* Final Direction */
			templeExit = 0;
			exitLevel(&templeExitFinal, exitDir);
			playSfx(SFX_POWER);
		}
	}
}

void stepFinalCastle(sprite, count)
	register Sprite* sprite;
	register int count;
{
	if (count == 2) {
		updateLevelTile(tile_pos(11, 1), 0x58, 0x0F, 0xC4);
		removeTriggers();
	}
	if (sprite->data == 1) {
		startBurners();
	}
}

void invokeScript(sprite, scriptId, data)
	register Sprite* sprite;
	register int scriptId, data;
{
	switch(scriptId) {
		case SCRIPT_INTRO:         stepIntro(sprite);       break;
		case SCRIPT_INIT_TEMPLE:   initTempleMaze(sprite);  break;
		case SCRIPT_CAVE_HALLWAY:  caveHallway(data);       break;
		case SCRIPT_WATER_BOSS:    waterBoss(sprite);       break;
		case SCRIPT_ROCK_ENTRANCE: rockEntrance(sprite, data); break;
		case SCRIPT_ROCK_GRASS:    rockGrass(data);         break;
		case SCRIPT_ROCK_BOSS:     rockBoss(data);          break;
		case SCRIPT_OPEN_EXIT:     openExit(sprite->data);  break;
		case SCRIPT_GRASS_TUNNEL:  grassTunnel(sprite, data); break;
		case SCRIPT_GRASS_WOODS:   grassWoods();            break;
		case SCRIPT_GRASS_BOSS:    grassBoss(sprite, data); break;
		case SCRIPT_REMOVE_CANNON: removeCannons();         break;
		case SCRIPT_START_BURNERS: startBurners();          break;
		case SCRIPT_ONOFF_BURNERS: toggleBurners(data);     break;
		case SCRIPT_MAIN_BONUS:    mainBonus(data);         break;
		case SCRIPT_TEMPLE_MAZE:   stepTempleMaze(sprite);  break;
		case SCRIPT_FINAL_CASTLE:  stepFinalCastle(sprite, data); break;
		case SCRIPT_FINAL_BOSS:    createBone(sprite);      break;
	}
}
