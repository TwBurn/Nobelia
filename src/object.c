#include <csd.h>
#include <sysio.h>
#include <ucm.h>
#include "game.h"
#include "graphics.h"
#include "audio.h"


Sprite* spriteList[SPRITE_COUNT];
u_char  spritePrio[SPRITE_COUNT];
/*                         0 1 2 3 4 5 6 7 8 9 A B C D E F */
u_char actionPriority[] = {0,2,2,2,2,2,2,2,3,3,3,3,1,1,1,1};

short spriteEmpty; /* Index of the first empty item */

void debugSprites()
{
	int i;
	for (i = 0; i < spriteEmpty; i++) {
		printf("%2d) %02X %04X\n", i, spriteList[i]->type, spriteList[i]->action);
	}
}

void debugObjects(level)
Level *level;
{
	int i;
	Sprite *sprite;
	for (i = 0; i < SPRITE_COUNT; i++) {
		sprite = &level->object[i];
		if (sprite->type == ST_NONE) continue;
		printf("%2d) %02X %04X\n", i, sprite->type, sprite->action);
	}
}

void clearSpriteList()
{
	int i;
	for (i = 0; i < SPRITE_COUNT; i++) {
		spriteList[i]->type = ST_NONE;
	}
	spriteEmpty = 0;
}

void initSpriteList()
{
	int i;
	Sprite *temp;
	for (i = 0; i < SPRITE_COUNT; i++) {
		spriteList[i] = &gameLevel.object[i];
	}

	/* Move non-empty objects to the start of the List */
	spriteEmpty = -1;
	for (i = 0; i < SPRITE_COUNT; i++) {
		if (spriteList[i]->type == ST_NONE) {
			/* Find first empty sprite */
			if (spriteEmpty == -1) spriteEmpty = i;
		}
		else {
			if (spriteEmpty >= 0) {
				/* Swap empty with current, increase empty */
				temp = spriteList[i];
				spriteList[i] = spriteList[spriteEmpty];
				spriteList[spriteEmpty] = temp;

				spriteEmpty++;
			}
		}
	}
}

Sprite* makeSprite(type, tile, frame, x, y)
	u_char type, tile, frame;
	short  x, y;
{
	Sprite *sprite;

	if (spriteEmpty >= SPRITE_COUNT) {
		return NULL;
	}
	else {
		sprite = spriteList[spriteEmpty++];
		sprite->type = type;
		sprite->tile = tile;
		sprite->frame = frame;
		sprite->x = x;
		sprite->y = y;
		
		return sprite;
	}
}

void walkAnim(sprite)
	Sprite *sprite;
{
	register u_short actDir  = sprite->action & 0x0f00;
	switch(actDir) {
		case DIR_LEFT:  sprite->frame = 0x05 + ((sprite->x >> 3) & 1); break;
		case DIR_RIGHT: sprite->frame = 0x09 + ((sprite->x >> 3) & 1); break;
		case DIR_UP:    sprite->frame = 0x0d + ((sprite->y >> 3) & 1); break;
		case DIR_DOWN:  sprite->frame = 0x01 + ((sprite->y >> 3) & 1); break;
	}
}

void stepBurner(sprite)
	Sprite *sprite;
{
	register u_short actType = sprite->action & 0xf000;
	if (actType != ACT_BURN) return;
	if (frameTick & 1) {
		sprite->frame = (frameTick >> 1) % 5;
		if (sprite->frame == 0) attackSprite(sprite->x, sprite->y, 0, AT_FIRE, 0);
	}
}

void stepTorch(sprite)
	Sprite *sprite;
{
	sprite->frame = (((u_int)sprite + frameTick) >> 2) % 4;
	if (sprite->frame == 3) sprite->frame = 1;
}

void stepPedestal(sprite)
	Sprite *sprite;
{
	if (sprite->action == ACT_ACTIVE) {
		stepTorch(sprite);
	}
	else {
		sprite->frame = 3;
	}
}

void activateBloom(sprite)
	Sprite *sprite;
{
	register u_short actType = sprite->action & 0xf000;
	register u_short actDir  = sprite->action & 0x0f00;
	register u_short actTime = sprite->action & 0x00ff;
	register u_short spriteTile = tile_index(sprite->x, sprite->y);

	if (sprite->desc == 0) { sprite->desc = gameLevel.tileMap[spriteTile].top; }

	if (actType == ACT_HIDDEN && actTime == 0) {
		sprite->action = ACT_HIDDEN | actDir | (sprite->data >> 8);
		switch (sprite->word) {
			case AT_FIRE: placeFires(sprite->x, sprite->y, actDir, 0xFF, 1, 0); break;
			case AT_PROJ: placeProjectiles(sprite->x, sprite->y, actDir); break;
		}
	}
}

void stepBloom(sprite)
	register Sprite *sprite;
{
	register u_short actType = sprite->action & 0xf000;
	register u_short actDir  = sprite->action & 0x0f00;
	register u_short actTime = sprite->action & 0x00ff;
	register u_short spriteTile = tile_index(sprite->x, sprite->y);
	register u_char move = gameLevel.tileMap[spriteTile].move & 0x0F; 

	if (actType == ACT_HIDDEN) {
		if (actTime == 0) {
			return;
		}
		else if (actTime == 1) {
			sprite->action = ACT_ACTIVE | actDir | (sprite->data & 0x00ff);
			sprite->frame = 0;
			gameLevel.tileMap[spriteTile].move = 0xE0 | move; /* BLOCK_PLAYER | BLOCK_PROJECTILE | BLOCK_FIRE */
		}
		else {
			sprite->action--;
		}
	}
	else {
		if (actTime == 0) {
			sprite->frame++;
			sprite->action |= sprite->data & 0x00ff;
		}
		else {
			sprite->action--;
		}

		if (sprite->frame == 4) {
			sprite->action = ACT_HIDDEN | actDir;
			gameLevel.tileMap[spriteTile].move = 0x90 | move; /* BLOCK_PLAYER | DESTROYABLE */
			gameLevel.tileMap[spriteTile].top = sprite->desc;
			drawLevelTile(spriteTile);
		}
	}
}

void stepAnim(sprite)
	Sprite *sprite;
{
	u_short actType = sprite->action & 0xF000;
	u_short actDir  = sprite->action & 0x0F00;
	u_short actTime = sprite->action & 0x00FF;
	u_short frameStep  = sprite->data & 0x00FF;
	u_short frameCount = sprite->data >> 8 & 0x7F;
	u_char  speed = sprite->desc >> 4;
	u_char  step  = sprite->desc & 0x0F;


	if (step && (actTime % step) == 0) {
		sprite->x += pix_dx(actDir) * speed;
		sprite->y += pix_dy(actDir) * speed;
	}
	if (frameStep && (actTime % frameStep) == 0) {
		sprite->frame += sprite->data & 0x8000 ? frameCount - 1 : 1;
		sprite->frame %= frameCount;
	}

	if (actTime) {
		sprite->action--;
	}
	else if (actType == ACT_LOOP) {
		sprite->action += (frameStep * frameCount) - 1;
	}
	else {
		sprite->type = ST_NONE;
	}
}

Sprite* findSprite(x, y)
	short x, y;
{
	int i;
	Sprite *sprite;
	for (i = spriteEmpty - 1; i >= 0; i--) {
		sprite = spriteList[i];
		if (sprite-> x == x && sprite->y == y) {
			switch(sprite->type) {
				case ST_SWITCH:
				case ST_PEDESTL:
				case ST_CHEST:
					return sprite;
			}
		}
	}

	return NULL;
}

void stepSprites()
{
	int i;
	Sprite *sprite;

	/* We loop through the list in reverse to make it possible to add new objects to the list but not call stepObject() on those yet */
	for (i = spriteEmpty - 1; i >= 0; i--) {
		sprite = spriteList[i];

		switch (sprite->type)
		{
			case ST_PLAYER:  stepPlayer(sprite);     break;
			case ST_ENEMY:   stepEnemy(sprite);      break;
			case ST_BOMB:    stepBomb(sprite);       break;
			case ST_FIRE:    stepFire(sprite);       break;
			case ST_CANNON:  stepCannon(sprite);     break;
			case ST_TORCH:   stepTorch(sprite);      break;
			case ST_CNBALL:  stepProjectile(sprite); break;
			case ST_ANIM:    stepAnim(sprite);       break;
			case ST_BURNER:  stepBurner(sprite);     break;
			case ST_BLOOM:   stepBloom(sprite);      break;
			case ST_PEDESTL: stepPedestal(sprite);   break;
			case ST_BONE:    stepBone(sprite);       break;
			case ST_SCRIPT:  invokeScript(sprite, sprite->action & 0x0fff, 0); break;
		}
		
		/* Check if we need to remove the current sprite */
		if (sprite->type == ST_NONE) {
			spriteEmpty--;
			if (i < spriteEmpty) {
				spriteList[i] = spriteList[spriteEmpty];
				spriteList[spriteEmpty] = sprite;
			}
		}
	}
}

void drawSpriteList()
{
	register int i;
	register Sprite *sprite;

	for (i = 0; i < spriteEmpty; i++) {
		spritePrio[i] = actionPriority[spriteList[i]->action >> 12];
	}

	for (i = spriteEmpty - 1; i >= 0; i--) {
		sprite = spriteList[i];
		if (spritePrio[i] == 1) drawSprite(sprite->tile + sprite->frame, sprite->x, sprite->y);
	}
	for (i = spriteEmpty - 1; i >= 0; i--) {
		sprite = spriteList[i];
		if (spritePrio[i] == 2) drawSprite(sprite->tile + sprite->frame, sprite->x, sprite->y);
	}
	for (i = spriteEmpty - 1; i >= 0; i--) {
		sprite = spriteList[i];
		if (spritePrio[i] == 3) drawSprite(sprite->tile + sprite->frame, sprite->x, sprite->y);
	}
}

void openExit(tileIndex)
	register u_short tileIndex;
{
	register int i;
	for (i = 0; i < EXIT_COUNT; i++) {
		if (gameLevel.exit[i].tile == tileIndex) {
			gameLevel.exit[i].type &= 0x7f;
		}
	}
}

void startBurners() {
	register int i;
	register Sprite *sprite;
	register u_short tileIndex;

	for (i = 0; i < spriteEmpty; i++) {
		sprite = spriteList[i];
		if (sprite->type == ST_BURNER) {
			sprite->action = ACT_BURN;
			tileIndex = tile_index(sprite->x, sprite->y);

			/* Unblock tile - Remove Top */
			gameLevel.tileMap[tileIndex].move &= 0x0f; 
			gameLevel.tileMap[tileIndex].top = 0;
			drawLevelTile(tileIndex);
		}
	}
}

void toggleBurners(state)
	int state;
{
	register int i;
	register Sprite *sprite;
	for (i = 0; i < spriteEmpty; i++) {
		sprite = spriteList[i];
		if (sprite->type == ST_BURNER) {
			if (state) sprite->action = ACT_BURN;
			else sprite->action = ACT_HIDDEN;
		}
	}
}

void removeCannons()
{
	register int i;
	register Sprite *sprite, *animObject;

	for (i = 0; i < spriteEmpty; i++) {
		sprite = spriteList[i];
		if (sprite->type == ST_CANNON) {
			sprite->type = ST_NONE;

			animObject = makeSprite(ST_ANIM, 0xC4, 0, sprite->x, sprite->y);
			if (animObject != NULL) {
				animObject->action = ACT_ACTIVE | 11; /* Active for 12 frames */
				animObject->data = 0x0403; /* 4 frames, 3 steps/frame*/
			}
		}
	}
}

void explodeUserBombs()
{
	register int i;
	register Sprite *sprite;

	if (playerState.BombType != ACT_USER) return;

	for (i = 0; i < spriteEmpty; i++) {
		sprite = spriteList[i];
		if (sprite->type == ST_BOMB && sprite->word == 1) {
			explodeBomb(sprite, DIR_NONE);
		}
	}
}

void removeUserBombs()
{
	int i;
	Sprite *sprite;

	for (i = 0; i < spriteEmpty; i++) {
		sprite = spriteList[i];
		if (sprite->type == ST_BOMB && sprite->word) {
			removeBomb(sprite);
		}
	}
}

void removeTriggers()
{
	int i;
	Sprite *sprite;

	for (i = 0; i < spriteEmpty; i++) {
		sprite = spriteList[i];
		if (sprite->type == ST_TRIGGER) {
			sprite->type = ST_NONE;
		}
	}
}

void killSprite(sprite) 
	Sprite* sprite;
{
	register u_short actType = sprite->action & 0xF000;
	if (actType == ACT_DEAD || actType == ACT_EXIT) return;

	sprite->data   = sprite->action;
	sprite->action = ACT_DEAD;
	sprite->desc = sprite->tile;
	sprite->tile = 0x80 - sprite->frame;

	playSfx(SFX_DEAD);
}

Sprite* findFire(x, y)
	register short x, y;
{
	register short dx, dy, i;
	register Sprite *sprite;

	for (i = 0; i < spriteEmpty; i++) {
		sprite = spriteList[i];
		dx = sprite->x - x; dx = (dx < 0) ? -dx : dx;
		dy = sprite->y - y; dy = (dy < 0) ? -dy : dy;

		if (dx >= 16 || dy >= 16) continue; /* Too far away */

		switch (sprite->type) {
			case ST_FIRE: return sprite;
		}
	}
	return NULL;
}

Sprite* attackSprite(x, y, dir, attackType, data)
	register short x, y, dir;
	register u_char attackType, data;
{
	register int i;
	register short dx, dy;
	register Sprite *sprite, *result = NULL;

	for (i = 0; i < spriteEmpty; i++) {
		sprite = spriteList[i];
		dx = sprite->x - x; dx = (dx < 0) ? -dx : dx;
		dy = sprite->y - y; dy = (dy < 0) ? -dy : dy;

		if (dx >= 16 || dy >= 16) continue; /* Too far away */

		switch (sprite->type) {
			case ST_PLAYER:
				playerHit(attackType, data);
				result = sprite;
			break;
			case ST_ENEMY:
				enemyHit(sprite, attackType, data);
				result = sprite;
			break;
			case ST_BOMB:
				explodeBomb(sprite, dir);
				result = sprite;
			break;
			case ST_CANNON:
				sprite->type = ST_NONE;
				result = sprite;
			break;
			case ST_TORCH:
				activateTorch(sprite);
				result = sprite;
			break;
			case ST_BLOOM:
				activateBloom(sprite);
				result = sprite;
			break;
		}

	}

	return result;
}

void setTrigger(trigger, state)
	register Sprite *trigger;
	register u_char state;
{
	register u_short tileIndex = tile_index(trigger->x, trigger->y);
	register u_char prevTile = gameLevel.tileMap[tileIndex].top;

	gameLevel.tileMap[tileIndex].top = trigger->tile;
	trigger->tile = prevTile;
	drawLevelTile(tileIndex);

	trigger->desc = state;
}

void checkTrigger(x, y, state)
	register u_short x, y;
	register u_char state;
{
	register int i, scriptId = 0, count = 0;
	Sprite *sprite, *trigger;

	for (i = 0; i < spriteEmpty; i++) {
		sprite = spriteList[i];
		if (sprite->type == ST_TRIGGER && sprite->x == x && sprite->y == y) {
			setTrigger(sprite, state);
			trigger = sprite;
			scriptId = sprite->word;
			break;
		}
	}

	if (scriptId == 0) return;
	playSfx(SFX_SWITCH);
	for (i = 0; i < spriteEmpty; i++) {
		sprite = spriteList[i];
		if (sprite->type == ST_TRIGGER && sprite->word == scriptId) {
			count += sprite->desc;
		};
	}

	invokeScript(trigger, scriptId, count);
}