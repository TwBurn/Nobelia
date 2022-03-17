#include <csd.h>
#include <sysio.h>
#include <stdio.h>
#include <ucm.h>
#include <memory.h>
#include <ERRNO.H>
#include <events.h>

#include "game.h"
#include "graphics.h"
#include "input.h"
#include "cdio.h"

#define HUD_X 248 /* Must be a multiple of 4 */
#define HUD_Y 18

#define TIMER_X 24
#define TIMER_Y 18

SaveFile gameSave;

Level* levelData;
Level gameLevel;

int gameState;
int nextState;

u_int gameTime    = 0; /* Game timer */
u_int templeValue = 0; /* Temple value */

PlayerState startState = {
	0,        /* BombMax */
	0,        /* BombCur */
	1,        /* BombSize */
	ACT_FIRE, /* BombType - ACT_FIRE or ACT_USER */
	0,        /* LevelNumber */
	0,        /* SpawnDir */
	0,        /* SpawnTile */
	0x00,     /* PowerUp */
	0,        /* Coins */
	0         /* ChestState */
};
PlayerState levelState;
PlayerState playerState;

u_int randState = 0xDEADC0DE;
u_int rand()
{
	u_int div = randState / 44488;	/* max: M / Q = A = 48,271 */
	u_int rem = randState % 44488;	/* max: Q - 1     = 44,487 */

	int s = rem * 48271; /* max: 44,487 * 48,271 = 2,147,431,977 = 0x7fff3629 */
	int t = div * 3399;	 /* max: 48,271 *  3,399 =   164,073,129 */
	int result = s - t;

	if (result < 0)
		result += 0x7fffffff;

	return randState = result;
}

u_char allowSave = 0;

char *saveFilename = "/nvr/nobelia";

/*
void debugBuffer(buffer, size)
	register u_char* buffer;
	int size;
{
	int i;
	for (i = 0; i < size; i++) {
		printf("%02X ", *buffer++);
		if ((i & 0xF) == 0xF) printf("\n");
	}
	if (i & 0xF) printf("\n");
}
*/

void saveGameCreate() {
	int file = creat(saveFilename);
	if (file == -1) return;
	
	write(file, &gameSave, sizeof(SaveFile));
	close(file);

	allowSave = 1;
}

void saveGame() {
	int file = open(saveFilename, WRITE_);
	if (file == -1) return;

	write(file, &gameSave, sizeof(SaveFile));
	close(file);
}

void loadGame()
{
	int file = open(saveFilename, READ_);

	if (file == -1) {
		/* Clear gameSave */
		memset(&gameSave, 0, sizeof(SaveFile));
		saveGameCreate(); /* Create Save File */
	}
	else {
		read(file, &gameSave, sizeof(SaveFile));
		close(file);

		allowSave = 1;
	}
}

void initGame()
{
	levelData = (Level*)ipSec->videoBuffer;
	loadGame();
}

int readLevelData(file)
{
	return read(file, levelData, LEVEL_COUNT * sizeof(Level));
}

void updateHudDemo()
{
	int displayLevel = playerState.LevelNumber - LEVEL_DEMO + 1;
	
	drawHudTile(0x0B, 0);
	drawHudTile(0x20 + displayLevel, 1);

	drawHudTile(0x31, 3);
	drawHudTile(0x20 + playerState.BombMax - playerState.BombCur, 4);

	if (playerState.BombType == ACT_FIRE) {
		drawHudTile(0x32, 6);
	}
	else {
		drawHudTile(0x33, 6);
	}
	
	drawHudTile(0x20 + playerState.BombSize, 7);
}

void updateHudGame()
{
	drawHudTile(0x31, 0);
	drawHudTile(0x20 + playerState.BombMax - playerState.BombCur, 1);

	if (playerState.BombType == ACT_FIRE) {
		drawHudTile(0x32, 2);
	}
	else {
		drawHudTile(0x33, 2);
	}
	
	drawHudTile(0x20 + playerState.BombSize, 3);

	drawHudTile(0x34 + ((playerState.Coins >> 0) & 1),  5);
	drawHudTile(0x34 + ((playerState.Coins >> 1) & 1),  6);
	drawHudTile(0x34 + ((playerState.Coins >> 2) & 1),  7);
	drawHudTile(0x34 + ((playerState.Coins >> 3) & 1),  8);
	drawHudTile(0x34 + ((playerState.Coins >> 4) & 1),  9);
	drawHudTile(0x34 + ((playerState.Coins >> 5) & 1), 10);
	drawHudTile(0x34 + ((playerState.Coins >> 6) & 1), 11);
}

void updateHud()
{
	if (playerState.LevelNumber >= LEVEL_DEMO) updateHudDemo();
	else updateHudGame();

	drawHud(HUD_X, HUD_Y, spDraw->videoBuffer);
	drawHud(HUD_X, HUD_Y, spActive->videoBuffer);
}

void updateTimer()
{
	drawTimerTime(gameTime);

	drawTimer(TIMER_X, TIMER_Y, spDraw->videoBuffer);
	drawTimer(TIMER_X, TIMER_Y, spActive->videoBuffer);
}

u_int nextGameTime = 0;
void incrementGameTime()
{
	if (gameTime >= MAX_TIME) return;

	if (++gameTime >= nextGameTime) {
		updateTimer();
		nextGameTime += GAME_FPS;
	}
}

void drawLevelTile(index)
	int index;
{
	LevelTile tile;
	register int position = tile_coord(index);

	tile = gameLevel.tileMap[index];


	if (tile.bottom)  drawTransTile(tile.bottom);
	if (tile.middle)  drawTransTile(tile.middle);
	if (tile.top)     drawTransTile(tile.top);

	drawTileBuffer(position, gameLevel.background);
}

void drawLevel()
{
	int i;
	
	clearTileBuffer(gameLevel.background);
	for (i = 0; i < TILE_COUNT; i++) {
		drawLevelTile(i);
	}
}
void saveLevel() {
	register int i = 0;
	Level *level = &levelData[playerState.LevelNumber];
	register int saveObject;
	register u_char objType;

	removeUserBombs();

	/* Save tile data */
	memcpy(level->tileMap, gameLevel.tileMap, sizeof(LevelTile) * TILE_COUNT);

	/* Save sprite data */
	for (i = 0; i < SPRITE_COUNT; i++) {
		objType = level->object[i].type;
		
		if (objType == ST_NONE) continue;
		if (objType != gameLevel.object[i].type) {
			level->object[i].type = ST_NONE;
			continue;
		}

		saveObject = 0;

		switch(objType) {
			case ST_CHEST:
			case ST_SWITCH:
			case ST_CANNON:
			case ST_BURNER:
			case ST_BLOOM:
			case ST_SCRIPT:
				saveObject = 1;
				break;
			case ST_ENEMY:
				if (gameLevel.object[i].action == ACT_DEAD) {
					saveObject = 1;
				}
				break;
		}

		if (saveObject) {
			memcpy(&level->object[i], &gameLevel.object[i], sizeof(Sprite));
		}
	}

	/* Save exit data */
	memcpy(level->exit, gameLevel.exit, sizeof(LevelExit) * EXIT_COUNT);

	/* Save game */
	if (playerState.LevelNumber < LEVEL_DEMO) {
		gameSave.elapsedTime = gameTime;
		gameSave.saveState   = playerState.ChestState | (templeValue << 20);
	
		if (allowSave) saveGame();
	}
}
u_int lastBorder = 0;
void loadLevel()
{
	gameLevel = levelData[playerState.LevelNumber];
	initSpriteList();
	initPlayer();

	/* Init Level */
	playerState.BombCur = 0;
	levelState = playerState;

	updateHud();
	updateTimer();

	/* Draw Tiles */
	if (gameLevel.border != lastBorder) {
		fillVideoBuffer(ipPrim->videoBuffer, gameLevel.border);
		lastBorder = gameLevel.border;
	}

	drawLevel();
	
	/* Draw Objects */
	clearSprites();
	drawSpriteList();
	
	swapSpriteBuffer();

	playMusic(gameLevel.musicChannel);
	
	gameState = GS_FADE_IN;
	nextState = GS_GAME_ACTIVE;
}

void levelComplete(nextLevel, spawnData)
	int nextLevel;
	u_int spawnData;
{
	if (nextLevel >= 0) { saveLevel(); }

	clearSpriteList();
	clearHud();
	clearTimer();
	clearSpriteBuffers();

	if (nextLevel < 0) {
		playerState = levelState;
		setIcf(ICF_MIN, ICF_MIN);
		gameState = GS_LEVEL_LOAD;
	}
	else if (nextLevel >= LEVEL_COUNT) {
		gameState = GS_FADE_OUT;
		nextState = GS_WIN_LOAD;
	}
	else {
		playerState.LevelNumber = nextLevel;
		playerState.SpawnDir  = spawnData >> 16;
		playerState.SpawnTile = spawnData & 0x0000FFFF;
		lastBorder = 0;

		gameState = GS_LEVEL_LOAD;
	}
}

void fadeIn()
{
	setIcf(curIcfA + FADE_SPEED, curIcfA + FADE_SPEED);
	if (curIcfA == ICF_MAX) {
		gameState = nextState;
		readInput1();
		readInput1();
	}
}

void fadeOut()
{
	setIcf(curIcfA - FADE_SPEED, curIcfA - FADE_SPEED);
	if (curIcfA == ICF_MIN) {
		gameState = nextState;
		readInput1();
		readInput1();
	}
}

#define PAUSE_FADE 0x1F
u_char allowResume;

void pauseGame() {
	int x, y;

	allowResume = 0;
	for (y = 0; y < 3; y++) {
		for (x = 0; x < 7; x++) {
			drawSprite(
				0x99 + (y << 4) + x,
				128  + (x << 4),
				96   + (y << 4)
			);
		}
	}

	updateHud();

	gameState = GS_GAME_PAUSE;
}

void resumeGame() {
	clearSprites();
	updateHud();
	swapSpriteBuffer();

	gameState = GS_GAME_RESUME;
}
void quitGame() {
	lastBorder = 0;
	titleLoad();
}

void gameStart() {
	register int levelNum, spriteNum, chestNum, chestState;
	Level  *level;
	Sprite *sprite;

	stopMusic();
	safeRead(readLevels);

	chestState = playerState.ChestState;

	/* Open saved chests */
	for (levelNum = 0; levelNum < LEVEL_DEMO; levelNum++) {
		level = &levelData[levelNum];
		for (spriteNum = 0; spriteNum < SPRITE_COUNT; spriteNum++) {
			sprite = &level->object[spriteNum];
			if (sprite->type == ST_CHEST) {
				chestNum = sprite->word >> 16;
				if (chestState & (1 << chestNum)) {
					openChest(sprite, level);
				}
			}
		}
	}

	gameState = GS_LEVEL_LOAD;
}


void gameNew()
{
	playerState = startState;
	lastBorder   = 0;
	gameTime     = 0;
	nextGameTime = 0;
	templeValue  = 0;
	titleLoaded  = 0;
}

void gameContinue()
{
	gameNew();
	if (gameSave.elapsedTime) {
		playerState.LevelNumber = LEVEL_CONTINUE;
		playerState.ChestState  = gameSave.saveState & 0x000FFFFF;
		gameTime     = gameSave.elapsedTime;
		nextGameTime = gameTime - (gameTime % GAME_FPS) + GAME_FPS;
		templeValue  = gameSave.saveState >> 20;
	}
}

void gameDemo()
{
	gameNew();
	playerState.LevelNumber = LEVEL_DEMO;
}

/* Game Loops */
void gameStep()
{
	incrementGameTime();
	clearSprites();
	stepSprites();
	playerInput();

	if (gameState == GS_GAME_ACTIVE) {
		drawSpriteList();
	}

	swapSpriteBuffer();
}

void gameResume()
{
	setIcf(ICF_MAX, curIcfB + FADE_SPEED);
	if (curIcfB == ICF_MAX) {
		gameState = GS_GAME_ACTIVE;
	}
}

void gamePause()
{
	u_short input = readInput1();

	if (curIcfB > PAUSE_FADE) {
		setIcf(ICF_MAX, curIcfB - FADE_SPEED);
	}

	if (allowResume) {
		if      (input & I_BUTTON1) resumeGame();
		else if (input & I_BUTTON2) quitGame();
	}
	else {
		if ((input & I_BUTTON_ANY) == 0) allowResume = 1;
	}
}

u_char gameAtt[] = {0,6,15,26,38,51,65,80,96,128};
void setGameAttenuation() {
	setAttenuation(
		gameAtt[gameSave.musicAttenuation],
		gameAtt[gameSave.sfxAttenuation]
	);
}

/* Main Loop */
void gameLoop()
{
	
	titleLoad();
	dc_ssig(videoPath, SIG_BLANK, 0);
	
	while (gameState) {
		if (!frameDone) continue; /* Wait for SIG_BLANK */

		frameDone = 0;
		switch (gameState) {
			case GS_FADE_IN:      fadeIn();    break;
			case GS_FADE_OUT:     fadeOut();   break;
			case GS_GAME_START:   gameStart(); break;
			case GS_GAME_ACTIVE:  gameStep();  break;
			case GS_LEVEL_LOAD:   loadLevel(); break;
			case GS_TITLE_ACTIVE: titleLoop(); break;
			case GS_TITLE_OUT:    titleOut();  break;
			case GS_TITLE_LOAD:   titleLoad(); break;
			case GS_TITLE_PICK:   titlePick(); break;
			case GS_GAME_PAUSE:   gamePause(); break;
			case GS_GAME_RESUME:  gameResume();break;
			case GS_HELP_ACTIVE:  helpLoop();  break;
			case GS_HELP_LOAD:    helpLoad();  break;
			case GS_SCORE_ACTIVE: scoreLoop(); break;
			case GS_SCORE_LOAD:   scoreLoad(); break;
			case GS_WIN_ACTIVE:   winLoop();   break;
			case GS_WIN_LOAD:     winLoad();   break;
		}
		rand();

		dc_ssig(videoPath, SIG_BLANK, 0);
	}
}

void discError()
{
	u_short input;
	u_short errorMode = 2;

	drawDiscError();

	dc_ssig(videoPath, SIG_BLANK, 0);

	while (errorMode) {
		input = readInput1();

		if (errorMode == 2) {
			if ((input & I_BUTTON_ANY) == 0) errorMode = 1;
		}
		else {
			if (input & I_BUTTON1) errorMode = 0;
		}

		while (!frameDone) {}; /* Wait for SIG_BLANK */
		frameDone = 0;
		dc_ssig(videoPath, SIG_BLANK, 0);
	}

	clearDiscError();
}

#define TEMPLE_LEVEL 25
#define TEMPLE_TILE_START tile_pos(5, 7)
#define TEMPLE_TILE_INDEX 0xCA

void updateTempleStatue(statueNum, dirNum)
	register int statueNum, dirNum;
{
	/* Set Head Tile of Temple Level */
	levelData[TEMPLE_LEVEL].tileMap[TEMPLE_TILE_START + statueNum * 4].top = TEMPLE_TILE_INDEX + 2 + dirNum;
	/* Set Statue Tile of Temple Level */
	levelData[TEMPLE_LEVEL].tileMap[TEMPLE_TILE_START + statueNum * 4 + TILES_X].top = TEMPLE_TILE_INDEX + (dirNum >> 1);
}

void updateIntroLevel()
{
	/* Remove Trunk */
	levelData[0].tileMap[tile_pos(10, 1)].top   = 0x00;
	levelData[0].tileMap[tile_pos(10, 1)].move &= 0x7f;
	levelData[0].tileMap[tile_pos(11, 1)].top   = 0x00;
	levelData[0].tileMap[tile_pos(11, 1)].move &= 0x7f;
	levelData[0].tileMap[tile_pos(12, 1)].top   = 0x00;
	levelData[0].tileMap[tile_pos(12, 1)].move &= 0x7f;

	/* Remove Door */
	levelData[0].tileMap[tile_pos(17, 1)].top   = 0x00;
	levelData[0].tileMap[tile_pos(17, 1)].move &= 0x7f;
	levelData[0].tileMap[tile_pos(17, 2)].top   = 0x00;
	levelData[0].tileMap[tile_pos(17, 2)].move &= 0x7f;
}