#include <csd.h>
#include <sysio.h>
#include <stdio.h>
#include "game.h"
#include "audio.h"
#include "graphics.h"
#include "input.h"

 /* Time player is invulnerable after being burned with cloak */
#define PLAYER_IFRAMES 64
/* Time player is vulnerable after iframes wear off */
#define PLAYER_VFRAMES 64

#define TILE_CLOAK 0x10
#define TILE_NORMAL 0x00

Sprite *player;
u_short buttonState;

void initPlayer()
{
	register int i;
	register u_short direction;
	player = NULL;
	for (i = 0; i < SPRITE_COUNT; i++) {
		if (gameLevel.object[i].type == ST_PLAYER)
			player = &gameLevel.object[i];	break;
	}

	if (player) {
		if (playerState.SpawnDir) {
			player->action = ACT_IDLE | playerState.SpawnDir;
			player->x = tile_x(playerState.SpawnTile);
			player->y = tile_y(playerState.SpawnTile);
		}

		direction = player->action & DIR_ALL;
		player->frame = dir_frame(direction);

		player->tile = playerState.PowerUp;

		buttonState = 0;
	}
}

/*
	Check if we can move to the tile with index '''tile''' using direction '''dir'''
*/
char canMoveTo(tile, dir)
	register u_short tile, dir;
{
	register char value = gameLevel.tileMap[tile].move;
	register char check = dir >> 8;
	return (value & 0x90) ? 0 : value & check;
}

char canPlaceBomb(tile, dir)
	register u_short tile, dir;
{
	register u_short revDir = dir_rev(dir);
	if (gameLevel.tileMap[tile].move & 0x90) {
		return 0;
	}
	else if (canMoveTo(tile + dir_tile(revDir), revDir)) {
		return 1;
	}
	else {
		return 0;
	}
}

void exitLevel(exit, direction)
	register LevelExit *exit;
	register u_short direction;
{
	register u_short exitDir =  exit->direction << 8;
	register u_short exitTime = 31;

	if (exitDir == 0) {
		exitDir = direction;
	}

	if (exit->type == 0x0F) {
		exitTime += 16;
		playSfx(SFX_TPORT);
	}
	else {
		playSfx(SFX_STEP);
	}

	player->action = ACT_EXIT | exitDir | exitTime;
	player->data = exit->type;
	player->desc = exit->next;
	player->word = exit->spawnTile + (exit->spawnDir << 24);
}

int checkExit(tile, direction)
	register u_short tile, direction;
{
	register int i;
	register LevelExit *exit;

	for (i = 0; i < EXIT_COUNT; i++) {
		exit = &gameLevel.exit[i];
		if (exit->tile == 0) {
			return 0;
		}
		else if (exit->tile == tile && (exit->type & 0x80) == 0) {
			exitLevel(exit, direction);
			return 1;
		}
	}

	return 0;
}

void actionPedestal(sprite)
	register Sprite *sprite;
{
	if (sprite->action == ACT_IDLE) {
		sprite->action = ACT_ACTIVE;
		player->action = ACT_FIRE | (player->action & 0x0f00) | 15; /* Fire animation - 16 frames */
		player->frame |= 3;
		playSfx(SFX_CRACK);
	}
}

void actionSwitch(sprite)
	Sprite *sprite;
{
	u_short actionDir  = sprite->action & 0x0f00;
	u_short actionSize = sprite->action & 0x00ff;
	u_short actionTile = sprite->data;
	u_short spriteTile = tile_index(sprite->x, sprite->y);

	Sprite *animObject;
	int i;

	player->action = ACT_FIRE | (player->action & 0x0f00) | 0x0008; /* Fire animation - 8 frames */
	player->frame |= 3;

	playSfx(SFX_SWITCH);

	for (i = 0; i < actionSize; i++) {
		gameLevel.tileMap[actionTile].move &= 0x1F; /* Clear BLOCK_PLAYER, BLOCK_FIRE and BLOCK_PROJECTILE flags */
		gameLevel.tileMap[actionTile].top = sprite->desc;
		drawLevelTile(actionTile);

		animObject = makeSprite(ST_ANIM, sprite->frame, 0, tile_x(actionTile), tile_y(actionTile));
		if (animObject != NULL) {
			animObject->action = ACT_ACTIVE | 11; /* Active for 12 frames */
			animObject->data = 0x0403; /* 4 frames, 3 steps/frame*/
		}

		actionTile += dir_tile(actionDir);
	}

	if (sprite->word) {
		invokeScript(sprite, sprite->word, 0);
	}

	gameLevel.tileMap[spriteTile].top = sprite->tile;
	drawLevelTile(spriteTile);

	sprite->type = ST_NONE;
}

void openChest(sprite, level)
	Sprite *sprite;
	Level  *level;
{
	register u_short spriteTile = tile_index(sprite->x, sprite->y);
	register u_int chestNum = sprite->word >> 16;

	switch (sprite->desc) {
		case ST_BOMB:
			playerState.BombMax++; break;
		case ST_FIRE:
			playerState.BombSize++; break;
		case ST_COIN:
			playerState.Coins |= 1 << ((sprite->word & 0x0000ffff) - 1); break;
		case ST_MAGIC:
			playerState.BombType = ACT_USER; break;
		case ST_CLOAK:
			playerState.PowerUp = TILE_CLOAK;
			break;
	}

	playerState.ChestState |= (1 << chestNum);

	if (level->tileMap[spriteTile].move & 0x0F) {
		level->tileMap[spriteTile].move &= 0x0F; /* Clear flags */
	}

	level->tileMap[spriteTile].top = sprite->data >> 8;


	sprite->type = ST_NONE;
}

void actionChest(sprite, playerDir)
	Sprite *sprite;
	u_short playerDir;
{
	Sprite* animObject;
	u_short spriteTile = tile_index(sprite->x, sprite->y);
	
	u_char animation = sprite->data & 0xFF;

	if (playerDir == DIR_DOWN) return;
	if (!canMoveTo(tile_index(player->x, player->y), dir_rev(playerDir))) return;

	openChest(sprite, &gameLevel);
	drawLevelTile(spriteTile);

	playSfx(SFX_POWER);
	
	player->action = ACT_FIRE | (player->action & 0x0f00) | 15; /* Fire animation - 16 frames */
	player->tile   = playerState.PowerUp;
	player->frame  |= 3;

	updateHud();

	animObject = makeSprite(ST_ANIM, animation, 0, sprite->x, sprite->y);
	if (animObject != NULL) {
		animObject->action = ACT_ACTIVE | DIR_UP | 31; /* Active for 32 frames */
		animObject->desc = 0x14;   /* Move 1 pixel every 4 frames */
		animObject->data = 0x0000; /* No step for animation */
	}
}

void playerAction()
{
	u_short actionDir = player->action & 0x0f00, revDir;
	Sprite *sprite;
	short x, y, tile;
	
	if (buttonState & I_BUTTON1) return;

	/* Calculate relative position and tile */
	x = player->x + dir_dx(actionDir);
	y = player->y + dir_dy(actionDir);

	sprite = findSprite(x, y);
	if (sprite) {
		switch (sprite->type) {
			case ST_SWITCH:  actionSwitch(sprite); break;
			case ST_CHEST:   actionChest(sprite, actionDir); break;
			case ST_PEDESTL: actionPedestal(sprite); break;
		}
		buttonState |= I_BUTTON1;
		return;
	}

	
	if (canPlaceBomb(tile_index(x, y), actionDir)) {
		if (placeBomb(x, y, playerState.BombType, playerState.BombSize, 1)) {
			player->action = ACT_FIRE | actionDir | 0x0007; /* Fire animation - 8 frames */
			player->frame |= 3;
			
			buttonState |= I_BUTTON1;
			return;
		}			
	}
	else {
		revDir = dir_rev(actionDir);
		x = player->x + dir_dx(revDir);
		y = player->y + dir_dy(revDir);
		if (canMoveTo(tile_index(x, y), dir_rev(actionDir))) {
			if (placeBomb(player->x, player->y, playerState.BombType, playerState.BombSize, 1)) {
				player->action = ACT_FIRE | actionDir | 0x0087; /* Fire animation - 8 frames, push back */
				player->frame |= 3;

				buttonState |= I_BUTTON1;
				return;
			}
		}
	}
	return;
}

void playerMove(direction)
	short direction;
{
	u_short tileIndex = tile_index(player->x, player->y);
	u_short actType = player->action & 0xf000;
	u_short actDir  = player->action & 0x0f00;
	u_short actTime = player->action & 0x00ff;

	player->frame = dir_frame(direction);
	tileIndex += dir_tile(direction);

	if (!canMoveTo(tileIndex, direction)) {
		player->action = ACT_IDLE | direction;
	}
	else if (checkExit(tileIndex, direction)) {
		playSfx(SFX_STEP);
	}
	else if (actType == ACT_ACTIVE || (actDir == direction && actTime == 0)) {
		player->action = ACT_MOVE | direction | 0x000f; /* Move 16 steps */
		player->data = direction;
		playSfx(SFX_STEP);
	}
	else if (actDir != direction) {
		player->action = ACT_IDLE | direction | 0x0007; /* Wait 8 frames before we can move */
	}
	
}

void playerInput()
{
	u_short input = readInput1();
	u_short actType = player->action & 0xf000;

	buttonState &= input;

	if (actType == ACT_IDLE || actType == ACT_ACTIVE) {
		if (input & I_BUTTON3) {
			pauseGame();
			return;
		}

		if      (input & I_BUTTON1) playerAction();
		else if (input & I_BUTTON2) explodeUserBombs();
		
		actType = player->action & 0xf000;
		if (actType == ACT_IDLE || actType == ACT_ACTIVE) {
			if      (input & I_LEFT)    playerMove(DIR_LEFT);
			else if (input & I_RIGHT)   playerMove(DIR_RIGHT);
			else if (input & I_UP)      playerMove(DIR_UP);
			else if (input & I_DOWN)    playerMove(DIR_DOWN);
		}
	}
	else if (actType == ACT_MOVE) {
		if (input & I_LEFT)         player->data = DIR_LEFT;
		else if (input & I_RIGHT)   player->data = DIR_RIGHT;
		else if (input & I_UP)      player->data = DIR_UP;
		else if (input & I_DOWN)    player->data = DIR_DOWN;
	}
	else if (actType == ACT_DEAD) {
		if (input & I_BUTTON1) levelComplete(-1, 0xFFFF);
		/*else if (input & I_BUTTON2) {
			player->action = player->data;
			player->tile = 0x00;
		}*/
	}
}

void playerHit(attackType, data)
	register u_char attackType, data;
{
	if (attackType == AT_FIRE && playerState.PowerUp == TILE_CLOAK) {
		if (player->word == 0) {
			player->word = PLAYER_VFRAMES + PLAYER_IFRAMES;
			playSfx(SFX_PLHIT);
			return;
		}
		else if (player->word > PLAYER_VFRAMES) {
			return;
		}
	}
	
	killSprite(player);
}

void stepPlayer(sprite)
	register Sprite *sprite;
{
	register u_short actType = sprite->action & 0xf000;
	register u_short actDir  = sprite->action & 0x0f00;
	register u_short actTime = sprite->action & 0x00ff;

	switch (actType) {
		case ACT_DEAD:
			if (curIcfB >= 0x10) setIcf(curIcfA, curIcfB - 1);
			return;
		case ACT_EXIT:
			setIcf(actTime * 2, actTime * 2);

			if (actTime >= 0x20 || sprite->data != 0x0F) {
				if (sprite->data || (actTime & 1)) {
					sprite->x += pix_dx(actDir);
					sprite->y += pix_dy(actDir);
				}
				walkAnim(sprite);
			}
			else if (actTime % 4 == 3) {
				/* 0x08, 0x00, 0x04, 0x0C */
				switch(sprite->frame & 0x0C) {
					case 0x00: sprite->frame = 0x04; break;
					case 0x04: sprite->frame = 0x0C; break;
					case 0x08: sprite->frame = 0x00; break;
					case 0x0C: sprite->frame = 0x08; break;
				}
			}

			if (actTime) {
				sprite->action--;
			}
			else {
				levelComplete(sprite->desc, sprite->word);
			}
			
			return;
		case ACT_IDLE: break;
		case ACT_FIRE:
			if (actTime & 0x80) {
				sprite->x -= 2 * pix_dx(actDir);
				sprite->y -= 2 * pix_dy(actDir);
				actTime &= 0x7F;
			}
		case ACT_ACTIVE:
			if (!actTime) {
				sprite->action = ACT_IDLE | actDir;
				sprite->frame &= 0xfc;
			}
			break;
		
		case ACT_MOVE:
			sprite->x += pix_dx(actDir);
			sprite->y += pix_dy(actDir);

			walkAnim(sprite);
			if (!actTime) {
				/*sprite->action = ACT_ACTIVE | actDir | 0x0004; /* Free to move for the next 4 frames */
				/*sprite->frame &= 0xfc;*/
			
				if (sprite->data) actDir = sprite->data;
				sprite->action = ACT_IDLE | actDir;
				sprite->frame  = dir_frame(actDir);
			}
			break;
	}

	if (sprite->word) {
		sprite->word--;
		if (sprite->word == 0) {
			sprite->tile = TILE_CLOAK;
		}
		else if (sprite->word <= PLAYER_VFRAMES) {
			sprite->tile = TILE_NORMAL;
		}
		else {
			sprite->tile = ((sprite->word & 1) == 0) ? TILE_CLOAK : TILE_NORMAL;
		}
	}

	if (actTime) sprite->action--;
}