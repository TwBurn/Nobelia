#include <csd.h>
#include <sysio.h>
#include <ucm.h>
#include "game.h"
#include "audio.h"

Object* objectList[OBJ_COUNT];
int olEmpty; /* Index of the first empty item */

void debugObjects()
{
	int i;
	for (i = 0; i < olEmpty; i++) {
		printf("%2d) %02X %04X\n", i, objectList[i]->type, objectList[i]->action);
	}
}

void clearObjectList()
{
	int i;
	for (i = 0; i < OBJ_COUNT; i++) {
		objectList[i]->type = OT_NONE;
	}
	olEmpty = 0;
}

void initObjectList()
{
	int i;
	Object *temp;
	for (i = 0; i < OBJ_COUNT; i++) {
		objectList[i] = &gameLevel.objects[i];
	}

	/* Move non-empty objects to the start of the List */
	olEmpty = -1;
	for (i = 0; i < OBJ_COUNT; i++) {
		if (objectList[i]->type == OT_NONE) {
			/* Find first empty object */
			if (olEmpty == -1) olEmpty = i;
		}
		else {
			if (olEmpty >= 0) {
				/* Swap empty with current, increase empty */
				temp = objectList[i];
				objectList[i] = objectList[olEmpty];
				objectList[olEmpty] = temp;

				olEmpty++;
			}
		}
	}
}

Object* makeObject(type, sprite, frame, x, y)
	u_char type, sprite, frame;
	short  x, y;
{
	Object *object;
	if (olEmpty >= OBJ_COUNT) {
		return NULL;
	}
	else {
		object = objectList[olEmpty++];
		object->type = type;
		object->sprite = sprite;
		object->frame = frame;
		object->x = x;
		object->y = y;

		return object;
	}
}

void stepPlayer(object)
	Object *object;
{
	u_short actType = object->action & 0xf000;
	u_short actDir  = object->action & 0x0f00;
	u_short actTime = object->action & 0x00ff;
	int f;
	
	switch (actType) {
		case ACT_DEAD: return;
		case ACT_IDLE: break;
		case ACT_FIRE:
		case ACT_ACTIVE:
			if (!actTime) {
				object->action = ACT_IDLE | actDir;
				object->frame &= 0xfc;
			}
			break;
		case ACT_MOVE:
			switch(actDir) {
				/*Move object, update frame */
				case DIR_LEFT:  object->frame = 0x05 + (((object->x--) >> 3) & 1); break;
				case DIR_RIGHT: object->frame = 0x09 + (((object->x++) >> 3) & 1); break;
				case DIR_UP:    object->frame = 0x0d + (((object->y--) >> 3) & 1); break;
				case DIR_DOWN:  object->frame = 0x01 + (((object->y++) >> 3) & 1); break;
			}
			if (!actTime) {
				object->action = ACT_ACTIVE | actDir | 0x0004; /* Free to move for the next 4 frames */
				object->frame &= 0xfc;
			}
			break;
	}
	if (actTime) object->action--;
}

void stepAnim(object)
	Object *object;
{
	u_short actDir  = object->action & 0x0F00;
	u_short actTime = object->action & 0x00FF;
	u_short frameStep  = object->data & 0x00FF;
	u_short frameCount = object->data >> 8;
	u_char  speed = object->desc >> 4;
	u_char  step  = object->desc & 0x0F;

	if (step && (actTime % step) == 0) {
		object->x += pix_dx(actDir) * speed;
		object->y += pix_dy(actDir) * speed;
	}
	if (frameStep && (actTime % frameStep) == 0) {
		object->frame = (object->frame + 1) % frameCount;
	}

	if (actTime) {
		object->action--;
	}
	else {
		object->type = OT_NONE;
	}
}

Object* findObject(x, y)
	short x, y;
{
	int i;
	Object *curObject;
	for (i = olEmpty - 1; i >= 0; i--) {
		curObject = objectList[i];
		if (curObject->type != OT_NONE && curObject-> x == x && curObject->y == y) {
			return curObject;
		}
	}

	return NULL;
}

void stepObjects()
{
	int i;
	Object *curObject;

	/* We loop through the list in reverse to make it possible to add new objects to the list but not call stepObject() on those yet */
	for (i = olEmpty - 1; i >= 0; i--) {
		curObject = objectList[i];

		switch (curObject->type)
		{
			case OT_PLAYER: stepPlayer(curObject);     break;
			case OT_BOMB:   stepBomb(curObject);       break;
			case OT_FIRE:   stepFire(curObject);       break;
			case OT_CANNON: stepCannon(curObject);     break;
			case OT_TORCH:  stepTorch(curObject);      break;
			case OT_CNBALL: stepProjectile(curObject); break;
			case OT_ANIM:   stepAnim(curObject);       break;
		}
		
		
		/* Check if we need to remove the current object */
		if (curObject->type == OT_NONE) {
			olEmpty--;
			if (i < olEmpty) {
				objectList[i] = objectList[olEmpty];
				objectList[olEmpty] = curObject;
			}
		}
	}
}

void drawObject(object)
	Object *object;
{
	switch(object->action & 0xf000)	{
		case ACT_HIDDEN: break;
		default:
			drawSprite(object->sprite + object->frame, object->x, object->y);
	}
}

void drawObjects()
{
	int i;
	for (i = 0; i < olEmpty; i++) {
		drawObject(objectList[i]);
	}
}

void explodeUserBombs()
{
	int i;
	Object *object;

	for (i = 0; i < olEmpty; i++) {
		object = objectList[i];
		if (object->type == OT_BOMB && ((object->action & 0xf000) == ACT_USER)) {
			explodeBomb(object, DIR_NONE);
		}
	}
}

void interactFireObject(x, y, dir)
	short x, y, dir;
{
	int i;
	Object *object;

	for (i = 0; i < olEmpty; i++) {
		object = objectList[i];
		switch (object->type) {
			case OT_PLAYER:
			case OT_SKELET:
				if (object->action == ACT_DEAD) {}
				/* Check distance to projectile */
				else if (abs(object->x - x) + abs(object->y - y) < 16) {
					object->data   = object->action;
					object->action = ACT_DEAD;
					object->sprite = 0xA0 - object->frame;
				}
				break;
			case OT_BOMB:
				if (x == object->x && y == object->y) {
					explodeBomb(object, dir);
				}
				break;
			case OT_CANNON:
				if (x == object->x && y == object->y) {
					object->type = OT_NONE;
				}
				break;
			case OT_TORCH:
				if (x == object->x && y == object->y) {
					activateTorch(object);
				}
				break;
		}
	}
}