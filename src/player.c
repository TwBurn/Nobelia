#include "sys.h"
#include "game.h"
#include "audio.h"
#include "input.h"

Object *player;

void initPlayer()
{
	int i;
	for (i = 0; i < OBJ_COUNT; i++) {
		if (gameLevel.objects[i].type == OT_PLAYER)
			player = &gameLevel.objects[i];	break;
	}
}

void actionSwitch(object)
	Object *object;
{
	u_short actionDir  = object->action & 0x0f00;
	u_short actionSize = object->action & 0x00ff;
	u_short actionTile = object->data;
	Object *animObject;
	int i;

	player->action = ACT_FIRE | (player->action & 0x0f00) | 0x0008; /* Fire animation - 8 frames */
	player->frame |= 3;

	for (i = 0; i < actionSize; i++) {
		gameLevel.tileMap[actionTile] = object->desc;
		drawTile(object->desc, tile_coord(actionTile), ipDraw->videoBuffer);

		animObject = makeObject(OT_ANIM, 0xEC, 0, tile_x(actionTile), tile_y(actionTile));
		if (animObject != NULL) {
			animObject->action = ACT_ACTIVE | 11; /* Active for 12 frames */
			animObject->data = 0x0403; /* 4 frames, 3 steps/frame*/
		}


		actionTile += dir_tile(actionDir);
	}

	drawTile(object->sprite, pixel_pos(object->x, object->y), ipDraw->videoBuffer);
	backgroundChanged = 1;

	object->type = OT_NONE;
}

void actionChest(object, playerDir)
	Object *object;
	u_short playerDir;
{
	Object* animObject;

	if (playerDir == DIR_DOWN) return;

	switch (object->desc) {
		case OT_BOMB:
			BombMax++; break;
		case OT_FIRE:
			BombSize++; break;
		case OT_MAGIC:
			BombType = ACT_USER; break;
	}
	playSfx(SFX_POWER);
	
	player->action = ACT_FIRE | (player->action & 0x0f00) | 16; /* Fire animation - 16 frames */
	player->frame |= 3;

	drawTile(object->sprite, pixel_pos(object->x, object->y), ipDraw->videoBuffer);
	backgroundChanged = 1;

	animObject = makeObject(OT_ANIM, 0xC7, object->desc, object->x, object->y);
	if (animObject != NULL) {
		animObject->action = ACT_ACTIVE | DIR_UP | 31; /* Active for 32 frames */
		animObject->desc = 0x14;   /* Move 1 pixel every 4 frames */
		animObject->data = 0x0000; /* No step for animation */
	}

	object->type = OT_NONE;
}

void playerAction()
{
	u_short actionDir = player->action & 0x0f00;

	Object *object;
	short x = player->x, y = player->y;
	
	/* Calculate relative position and tile */
	switch (actionDir) {
		case DIR_LEFT:  x -= 16; break;
		case DIR_RIGHT: x += 16; break;
		case DIR_UP:    y -= 16; break;
		case DIR_DOWN:  y += 16; break;
	}

	object = findObject(x, y);

	if (object) {
		switch (object->type) {
			case OT_SWITCH: actionSwitch(object); break;
			case OT_CHEST:  actionChest(object, actionDir);  break;
		}
	}
	if (canMoveTo(tile_pos(x, y), actionDir) && placeBomb(x, y, BombType, BombSize)) {
		player->action = ACT_FIRE | actionDir | 0x0008; /* Fire animation - 8 frames */
		player->frame |= 3;
	}
}

void playerMove(direction)
	short direction;
{
	u_short tileIndex = tile_pos(player->x, player->y);
	u_short actType = player->action & 0xf000;
	u_short actDir  = player->action & 0x0f00;
	u_short actTime = player->action & 0x00ff;

	switch (direction) {
		case DIR_LEFT:
			tileIndex -= 1;
			player->frame = 0x04;
			break;
		case DIR_RIGHT:
			tileIndex += 1;
			player->frame = 0x08;
			break;
		case DIR_UP:
			tileIndex -= TILES_X;
			player->frame = 0x0C;
			break;
		case DIR_DOWN:
			tileIndex += TILES_X;
			player->frame = 0x00;
			break;
	}

	if (!canMoveTo(tileIndex, actDir)) {
		player->action = ACT_IDLE | direction;
	}
	else if (actType == ACT_ACTIVE || (actDir == direction && actTime == 0)) {
		player->action = ACT_MOVE | direction | 0x000f; /* Move 16 steps */
		playSfx(SFX_STEP);
	}
	else if (actDir != direction) {
		player->action = ACT_IDLE | direction | 0x0008; /* Wait 8 frames before we can move */
	}
	
}

void playerInput()
{
	u_short input = readInput1();
	u_short actType = player->action & 0xf000;

	if (player->y < 32) { 
		setICF(player->y * 2);
		if (player->y == 0) {
			levelComplete(1);
		}
		else if (player->y == 16) {
			player->action = ACT_MOVE | DIR_UP | 0x000f;
		}
	}

	if (actType == ACT_IDLE || actType == ACT_ACTIVE) {
		if      (input & I_BUTTON1) playerAction();
		else if (input & I_BUTTON2) explodeUserBombs();
		else if (input & I_LEFT)    playerMove(DIR_LEFT);
		else if (input & I_RIGHT)   playerMove(DIR_RIGHT);
		else if (input & I_UP)      playerMove(DIR_UP);
		else if (input & I_DOWN)    playerMove(DIR_DOWN);
	}
	else if (actType == ACT_DEAD) {
		if (input & I_BUTTON1) levelComplete(0);
		else if (input & I_BUTTON2) {
			player->action = player->data;
			player->sprite = 0x80;
		}
	}
}
