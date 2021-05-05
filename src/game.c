#include <csd.h>
#include <sysio.h>
#include <stdio.h>
#include <ucm.h>

#include "game.h"
#include "input.h"

#define SIG_BLANK 0xB00

#define FADE_SPEED 2

#define GS_TITLE_LOAD   0x00010000
#define GS_TITLESCREEN  0x00020000
#define GS_LOAD_LEVEL   0x00030000
#define GS_GAME_ACTIVE  0x00040000
#define GS_FADE_IN      0x00F10000
#define GS_FADE_OUT     0x00F00000

int gameState;
int nextState;

u_int gameTick = 0;

Level gameLevel;
u_char backgroundChanged;

/*
	Last 4 bits: Can move into from direction; ie DIR_DOWN -> top is open
	Bit 5: Fire can be on square
	Bit 6: Projectile can pass it
*/
char tileMove[] = {
	0x3f, 0x3f, 0x3b, 0x3f, 0xbf, 0x37, 0x3f, 0x37, 0x3f, 0x3e, 0x3f, 0x3e, 0x3d, 0x3f, 0x3d, 0x3f, 
	0x3f, 0x3b, 0x3f, 0x3e, 0x3d, 0x3f, 0x37, 0x3f, 0x35, 0x36, 0x39, 0x3a, 0x3f, 0x3f, 0x3f, 0x3f, 
	0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 
	0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x20, 0x20, 0x20, 0x20,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

u_int randState = 0xDEADBEEF;
u_int rand()
{
	u_int div = randState / 44488;	/* max: M / Q = A = 48,271 */
	u_int rem = randState % 44488;	/* max: Q - 1     = 44,487 */

	int s = rem * 48271;	/* max: 44,487 * 48,271 = 2,147,431,977 = 0x7fff3629 */
	int t = div * 3399;	/* max: 48,271 *  3,399 =   164,073,129 */
	int result = s - t;

	if (result < 0)
		result += 0x7fffffff;

	return randState = result;
}

char  StartBombMax;
char  StartBombSize;
short StartBombType;

void loadLevelFile(filename)
    u_char *filename;
{
    FILE *file = fopen(filename, "r");
	printf(file);
	fread(&gameLevel, 1, sizeof(Level), file);
	fclose(file);
}

int levelNumber;
char levelName[12];

void updateHud()
{
	drawHudTile(levelNumber / 10, 0);
	drawHudTile(levelNumber % 10, 1);

	drawHudTile(0x0B, 3);
	drawHudTile(BombMax - BombCur, 4);

	if (BombType == ACT_FIRE) {
		drawHudTile(0x0C, 6);
	}
	else {
		drawHudTile(0x0D, 6);
	}
	
	drawHudTile(BombSize, 7);

	drawHud(0x00F6, 0x0012, spDraw->videoBuffer);
}

void loadLevel()
{
	/* Load Level File */
	sprintf(levelName, "LEVEL%02d.NBL\n", levelNumber);
	loadLevelFile(levelName);
	initObjectList();
	initPlayer();

	/* Init Level */
	backgroundChanged = 1;

	StartBombMax  = BombMax;
	StartBombSize = BombSize;
	StartBombType = BombType;
	BombCur = 0;

	/* Draw Tiles */
	fillVideoBuffer(ipDraw->videoBuffer, 0x4F4F4F4F);
	loadTilemap(ipDraw->videoBuffer, gameLevel.tileMap);
	swapImageBuffer();
	
	/* Draw Objects */
	clearSprites();
	drawObjects();
	updateHud();
	swapSpriteBuffer();

	streamMusic(1);
	
	gameState = GS_FADE_IN;
	nextState = GS_GAME_ACTIVE;
}

void levelComplete(delta)
	int delta;
{
	stopMusic();
	clearObjectList();
	clearHud();
	clearSpriteBuffers();

	levelNumber += delta;
	if (delta == 0) {
		BombMax  = StartBombMax;
		BombSize = StartBombSize;
		BombType = StartBombType;
	}

	if (levelNumber > LEVEL_COUNT) {
		gameState = GS_TITLE_LOAD;
	}
	else {
		gameState = GS_LOAD_LEVEL;
	}
}



void gameStep()
{
	/* Copy active background to draw background - not the most efficient way but it works for now */
	if (backgroundChanged) {
		memcpy(ipDraw->videoBuffer, ipActive->videoBuffer, SCREEN_WIDTH * SCREEN_HEIGHT);
		backgroundChanged = 0;
	}

	clearSprites();
	stepObjects();
	playerInput();

	if (gameState == GS_GAME_ACTIVE) {
		updateHud();
		drawObjects();
	}

	swapSpriteBuffer();

	if (backgroundChanged) {
		/* Swap image buffer - on the next frame sync the background buffers */
		swapImageBuffer();
	}
}

void titleLoad()
{
	setICF(ICF_MIN);
	gameState = GS_FADE_IN;
	nextState = GS_TITLESCREEN;
	loadData();
	swapImageBuffer();
	playMusic(0);
}

void titleLoop()
{
	u_short input = readInput1();
	if (input & I_BUTTON1) {
		stopMusic();

		levelNumber = START_LEVEL;
		BombMax     = START_BMAX;
		BombSize    = START_BSIZE;
		BombType    = START_BTYPE;

		gameState = GS_FADE_OUT;
		nextState = GS_LOAD_LEVEL;
	}
}

void gameStart()
{
	titleLoad();

	dc_ssig(videoPath, SIG_BLANK, 0);
	while (1) {
		
	}
}

void fadeIn()
{
	setICF(curICF + FADE_SPEED);
	if (curICF == ICF_MAX) {
		gameState = nextState;
	}
}

void fadeOut()
{
	setICF(curICF - FADE_SPEED);
	if (curICF == ICF_MIN) {
		gameState = nextState;
	}
}

void handleGameSignal(sigCode)
{
	if (sigCode == SIG_BLANK) {
		switch (gameState) {
			case GS_FADE_IN:     fadeIn();    break;
			case GS_FADE_OUT:    fadeOut();   break;
			case GS_GAME_ACTIVE: gameStep();  break;
			case GS_LOAD_LEVEL:  loadLevel(); break;
			case GS_TITLESCREEN: titleLoop(); break;
			case GS_TITLE_LOAD:  titleLoad(); break;
		}
		gameTick++;
		rand();
		/*readAudio();*/
		dc_ssig(videoPath, SIG_BLANK, 0);
	}
}

/*
	Check if we can move to the tile with index '''tile''' using direction '''dir'''
*/
char canMoveTo(tile, dir)
	u_short tile, dir;
{
	char check = dir >> 8;
	return tileMove[gameLevel.tileMap[tile]] & check;
}

