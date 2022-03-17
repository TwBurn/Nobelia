#include <csd.h>
#include <sysio.h>
#include <stdio.h>
#include <ucm.h>
#include <memory.h>

#include "cdio.h"
#include "audio.h"
#include "game.h"
#include "graphics.h"
#include "input.h"

#define TITLE_START_X 128
#define TITLE_START_Y 112
#define TITLE_STEP     21

#define INSTRUCT_SIZE 14336
#define INSTRUCT_TOP (6*16*128)

#define INPUT_DELAY 6
char inputDelay;

int titleHighlight[] = {cp_cbnk(2), cp_clut(19, 0x73, 0x17, 0x2D), cp_clut(20, 0xB4, 0x20, 0x2A), cp_clut(21, 0xE8, 0x6A, 0x73)};
int titleNormal[]    = {cp_cbnk(2), cp_clut(19, 0x16, 0x51, 0x91), cp_clut(20, 0x1B, 0x68, 0xCC), cp_clut(21, 0x1F, 0xA1, 0xED)};

Sprite *titleSprite;
Sprite *flameSprite;

int titleItem = 1;
int titleCount = 5;

int lastHighlight;

char *instructBuffer;

void initTitle()
{
	instructBuffer = (char*)srqcmem(INSTRUCT_SIZE, VIDEO1);
}

int readInstructions(file)
	int file;
{
	return read(file, instructBuffer, INSTRUCT_SIZE);
}

void nextItem() {
	if (titleItem >= titleCount) return;

	titleItem++;
	titleSprite->action = ACT_MOVE | DIR_DOWN | (TITLE_STEP - 1);
	playSfx(SFX_STEP);
}

void prevItem() {
	if (titleItem <= 1) return;

	titleItem--;
	titleSprite->action = ACT_MOVE | DIR_UP | (TITLE_STEP - 1);
	playSfx(SFX_STEP);
}

void pickNewGame() {
	u_short input = readInput1();
	if (input & I_BUTTON2) gameDemo();
	else gameNew();
	gameState = GS_GAME_START;
}
void pickContinue() {
	gameContinue();
	gameState = GS_GAME_START;
}
void pickShowScore() {
	gameState = GS_SCORE_LOAD;
}
void pickShowHelp() {
	gameState = GS_HELP_LOAD;
}
void pickQuitGame() {
	gameState = GS_QUIT_GAME;
}

void titlePick() {
	gameState = GS_TITLE_LOAD;
	switch (titleItem) {
		case 1:  pickNewGame();   break;
		case 2:  pickContinue();  break;
		case 3:  pickShowScore(); break;
		case 4:  pickShowHelp();  break;
		case 5:  pickQuitGame();  break;
	}
}

int pickItem() {
	titleSprite->action = ACT_MOVE | DIR_RIGHT | ICF_MAX;
	gameState = GS_TITLE_OUT;
	nextState = GS_TITLE_PICK;
	playSfx(SFX_POWER);
}

void clearHighlight() {
	if (lastHighlight) {
		dc_nop(videoPath, lctB, 2 * lastHighlight,              1, 1, 4);
		dc_nop(videoPath, lctB, 2 * (lastHighlight + TITLE_STEP), 1, 1, 4);
		lastHighlight = 0;
	}
}

void titleDraw() {
	int curHighlight = titleSprite->y + OFFSET_Y - 2 - lineSkip;

	if (curHighlight != lastHighlight) {
		clearHighlight();

		dc_wrlct(videoPath, lctB, 2 * curHighlight, 1, 1, 4, titleHighlight);
		dc_wrlct(videoPath, lctB, 2 * (curHighlight + TITLE_STEP), 1, 1, 4, titleNormal);

		lastHighlight = curHighlight;
	}

	clearSprites();
	stepSprites();
	drawSpriteList();
	swapSpriteBuffer();
}

void titleOut() {
	titleDraw();
	
	setIcf(titleSprite->action & 0x00FF, titleSprite->action & 0x00FF);
	if (curIcfA == ICF_MIN) {
		clearHighlight();
		clearSpriteList();
		clearSpriteBuffers();

		gameState = nextState;
	}
}

void makeTitleSprite(x, y)
	short x, y;
{
	titleSprite = makeSprite(ST_PLAYER, 0x00, 0x00, x, y);
	titleSprite->action = ACT_IDLE | DIR_DOWN;
	titleSprite->desc = 0;
	titleSprite->data = 0;
	titleSprite->word = 0;
}

void makeFlameSprite()
{
	flameSprite = makeSprite(ST_ANIM, 0xDC, 0x00, 246, 33);
	if (flameSprite) {
		flameSprite->action = ACT_LOOP;
		flameSprite->desc = 0;
		flameSprite->data = 0x0406;
	}
}

void titleLoad()
{
	setIcf(ICF_MIN, ICF_MIN);
	gameState = GS_FADE_IN;
	nextState = GS_TITLE_ACTIVE;

	if (!titleLoaded) {
		stopMusic();
		safeRead(readTitle);
		playMusic(0);
		setGameAttenuation();
	}

	showPrimaryImageBuffer();

	initSpriteList();
	clearSpriteList();
	clearSpriteBuffers();
		
	/*titleItem = 1;*/
	lastHighlight = 0;

	makeTitleSprite(TITLE_START_X, TITLE_START_Y + (titleItem - 1) * TITLE_STEP);
	titleSprite->action = ACT_IDLE | DIR_DOWN;
	
	makeFlameSprite();

	titleDraw();
}

void titleLoop()
{
	u_short input = readInput1();
	u_short actType = titleSprite->action & 0xf000;

	titleDraw();

	if (actType == ACT_IDLE || actType == ACT_ACTIVE) {
		if      (input & I_BUTTON1) pickItem();
		else if (input & I_DOWN)    nextItem();
		else if (input & I_UP)      prevItem();
	}
}

#define SCORE_WIDTH 25
#define SCORE_STEP 60
u_char scoreBoard[SCORE_WIDTH * SCORE_COUNT];
u_char scoreShow;
short  scoreDraw;

void fillScoreRow(score, num)
	Score score;
	int num;
{
	register u_char* boardRow = scoreBoard + num * SCORE_WIDTH;
	int i, time, part;

	time = score.time;

	/* Part 1: Header | Number <dot> */
	boardRow[0] = 0x21 + num;
	boardRow[1] = 0x2A;

	boardRow[2] = 0x3F;

	if (time) {
		/* Part 2: Name */
		boardRow[3] = score.name_0;
		boardRow[4] = score.name_1;
		boardRow[5] = score.name_2;
	
		boardRow[6] = 0x3F;
		/* Part 3: Time   | H:MM:SS.ff */
		/*Fraction*/
		part = 2 * (time % 50); time = time / 50;
		boardRow[16] = 0x20 + part % 10;
		boardRow[15] = 0x20 + part / 10;
		boardRow[14] = 0x2A;
		/*Seconds*/
		part = time % 60; time = time / 60;
		boardRow[13] = 0x20 + part % 10;
		boardRow[12] = 0x20 + part / 10;
		boardRow[11] = 0x2B;
		/*Minutes*/
		part = time % 60; time = time / 60;
		boardRow[10] = 0x20 + part % 10;
		boardRow[9] = part < 10 ? 0x3f : (0x20 + part / 10);
		/*Hours*/
		boardRow[8] = time ? 0x2B : 0x3F;
		boardRow[7] = time ? 0x20 + time : 0x3F;

		boardRow[17] = 0x3f;
		if (scoreShow == 2) {
			/* Demo | B<Bombs><space>F<Fire> */
			boardRow[18] = 0x31;
			boardRow[19] = 0x20 + score.bomb;
			boardRow[20] = 0x3f;
			boardRow[21] = (score.fire >= 8) ? 0x32 : 0x33;
			boardRow[22] = 0x20 + (score.fire & 0x07);
			boardRow[23] = 0x3f;
			boardRow[24] = 0x3f;
		}
		else {
			boardRow[18] = score.fire & 1 ? 0x35 : 0x34;
			boardRow[19] = score.fire & 2 ? 0x35 : 0x34;
			boardRow[20] = score.fire & 4 ? 0x35 : 0x34;
			boardRow[21] = score.fire & 8 ? 0x35 : 0x34;
			boardRow[22] = score.bomb & 1 ? 0x35 : 0x34;
			boardRow[23] = score.bomb & 2 ? 0x35 : 0x34;
			boardRow[24] = score.bomb & 4 ? 0x35 : 0x34;
		}
		
	}
	else {
		/* No Score */
		boardRow[3] = 0x1f; boardRow[4] = 0x1f; boardRow[5] = 0x1f;
		
		boardRow[6] = 0x3f;

		boardRow[7]  = 0x1f; boardRow[8] = 0x2B;
		boardRow[9]  = 0x1f; boardRow[10] = 0x1f; boardRow[11] = 0x2B;
		boardRow[12] = 0x1f; boardRow[13] = 0x1f; boardRow[14] = 0x2A;
		boardRow[15] = 0x1f; boardRow[16] = 0x1f;

		boardRow[17] = 0x3F;

		if (scoreShow == 2) {
			boardRow[18] = 0x31;
			boardRow[19] = 0x1F;
			boardRow[20] = 0x3f;
			boardRow[21] = 0x33;
			boardRow[22] = 0x1F;
			boardRow[23] = 0x3f;
			boardRow[24] = 0x3f;
		}
		else {
			boardRow[18] = 0x34;
			boardRow[19] = 0x34;
			boardRow[20] = 0x34;
			boardRow[21] = 0x34;
			boardRow[22] = 0x34;
			boardRow[23] = 0x34;
			boardRow[24] = 0x34;
		}
	}

	scoreDraw = 0;
	
}

void fillScoreBoard() {
	Score *score;
	int i;

	switch(scoreShow) {
		case 0: score = (Score*)gameSave.gameScore; break;
		case 1: score = (Score*)gameSave.fullScore; break;
		case 2: score = (Score*)gameSave.demoScore; break;
	}

	for (i = 0; i < SCORE_COUNT; i++) {
		fillScoreRow(score[i], i);
	}

}

void scoreLoad()
{
	showSecondaryImageBuffer();
	clearRect(ipSec->videoBuffer, 3*16, 8*16 - 2, 18*16, 6*16, 59);
	copyRect(instructBuffer + INSTRUCT_TOP + 00, ipSec->videoBuffer,  65, 130, 44, 16, 128);
	copyRect(instructBuffer + INSTRUCT_TOP + 44, ipSec->videoBuffer, 125, 130, 36, 16, 128);
	copyRect(instructBuffer + INSTRUCT_TOP + 80, ipSec->videoBuffer, 185, 130, 40, 16, 128);

	initSpriteList();
	clearSpriteList();
	clearSpriteBuffers();

	scoreShow = 0;
	fillScoreBoard();

	makeTitleSprite(40, 110);
	titleSprite->tile = 0x10;
	titleSprite->action = ACT_IDLE | DIR_DOWN | (SCORE_STEP - 1);

	makeFlameSprite();

	drawSpriteList();
	swapSpriteBuffer();

	gameState = GS_SCORE_ACTIVE;
}

void scoreExit() {
	gameState = GS_FADE_OUT;
	nextState = GS_TITLE_LOAD;
}

void scoreNext() {
	if (scoreShow >= 2) return;
	titleSprite->action = ACT_MOVE | DIR_RIGHT | (SCORE_STEP - 1);
	scoreShow++;
	fillScoreBoard();
}

void scorePrev() {
	if (scoreShow == 0) return;
	titleSprite->action = ACT_MOVE | DIR_LEFT | (SCORE_STEP - 1);
	scoreShow--;
	fillScoreBoard();
}

#define SCORE_X 80
#define SCORE_Y 150

void scoreDrawFrame() {
	short x, y, tile;
	for (y = 0; y < 8; y++) {
		x = (scoreDraw - y) / 2;
		if (x >= 0 && x < SCORE_WIDTH) {
			clearRect(ipSec->videoBuffer,  SCORE_X + 8 * x, SCORE_Y + 8 * y, 8, 8, 59);
		}
		
		x--;
		if (x >= 0 && x < SCORE_WIDTH) {
			tile = scoreBoard[SCORE_WIDTH * y + x];
			copyRect(
				spriteSheet + SCORE_FONT + ((tile & 0x20) << 6) + ((tile & 0x1f) << 3),
				ipSec->videoBuffer, 
				SCORE_X + 8 * x, SCORE_Y + 8 * y, 8, 8, 256
			);
		}
	}

	scoreDraw++;
}

void scoreLoop() {
	u_short input = readInput1();
	u_short actType = titleSprite->action & 0xf000;
	u_short actTime = titleSprite->action & 0x00ff;
	
	if (curIcfA != ICF_MAX) setIcf(curIcfA + FADE_SPEED, curIcfB + FADE_SPEED);

	if (actTime) {
		scoreDrawFrame();
	}
	else {
		if      (input & I_BUTTON1) scoreExit();
		else if (input & I_RIGHT)   scoreNext();
		else if (input & I_LEFT)    scorePrev();
		titleSprite->frame = 0;
	}

	clearSprites();
	stepSprites();
	drawSpriteList();
	swapSpriteBuffer();
}

#define HELP_X 256
#define HELP_Y 132
void helpChar(x, y, chr)
	int x, y, chr;
{
	copyRect(
		spriteSheet + SCORE_FONT + ((chr & 0x20) << 6) + ((chr & 0x1f) << 3),
		ipSec->videoBuffer, 
		HELP_X + 8 * x, HELP_Y + 8 * y, 8, 8, 256
	);
}

u_char helpVolumeSelect = 0;

void helpVolumeUpdate() {
	clearRect(ipSec->videoBuffer, HELP_X + 8 * 8, HELP_Y + 2 * 8, 16, 16, 59);

	helpChar(8, 2, 0x29 - gameSave.musicAttenuation);
	helpChar(8, 3, 0x29 - gameSave.sfxAttenuation);
	helpChar(9, 2 + helpVolumeSelect, 0x38);
}


void helpVolume()
{
	/* VOLUME */
	helpChar(2, 0, 0x15);
	helpChar(3, 0, 0x0E);
	helpChar(4, 0, 0x0B);
	helpChar(5, 0, 0x14);
	helpChar(6, 0, 0x0C);
	helpChar(7, 0, 0x04);

	/* MUSIC */
	helpChar(1, 2, 0x0C);
	helpChar(2, 2, 0x14);
	helpChar(3, 2, 0x12);
	helpChar(4, 2, 0x08);
	helpChar(5, 2, 0x02);
	helpChar(6, 2, 0x2b);

	/* SFX */
	helpChar(3, 3, 0x12);
	helpChar(4, 3, 0x05);
	helpChar(5, 3, 0x17);
	helpChar(6, 3, 0x2b);

	helpVolumeUpdate();
}

void helpVersion()
{
	helpChar(4, 10, 0x15); /*V*/
	
	helpChar(5, 10, 0x21); /*1*/
	helpChar(6, 10, 0x2A); /*.*/
	helpChar(7, 10, 0x20); /*0*/
	helpChar(8, 10, 0x21); /*1*/
	helpChar(9, 10, 0x0F); /*P*/
}

void helpLoad()
{
	Sprite *sprite;
	showSecondaryImageBuffer();
	clearRect(ipSec->videoBuffer, 3*16, 8*16 - 2, 18*16, 6*16, 59);
	copyRect(instructBuffer, ipSec->videoBuffer, 80, 130, 128, 6*16, 128);
	
	inputDelay = 0;

	helpVolume();

	/* Version Number */
	helpVersion();
	
	initSpriteList();
	clearSpriteList();
	clearSpriteBuffers();

	makeFlameSprite();

	/* Player Animation */
	sprite = makeSprite(ST_ANIM, 0x00, 0x00, 44, 110);
	if (sprite) {
		sprite->action = ACT_LOOP;
		sprite->desc = 0;
		sprite->data = 0x100C;
	}

	/* Bomb Animation */
	sprite = makeSprite(ST_ANIM, 0xD0, 0x00, 44, 130);
	if (sprite) {
		sprite->action = ACT_LOOP;
		sprite->desc = 0;
		sprite->data = 0x8518;
	}

	/* Magic Bomb Animation */
	sprite = makeSprite(ST_ANIM, 0xD6, 0x01, 44, 148);
	if (sprite) {
		sprite->action = ACT_LOOP;
		sprite->desc = 0;
		sprite->data = 0x0208;
	}

	/* Switch/Chest Animation */
	sprite = makeSprite(ST_ANIM, 0x81, 0x00, 44, 166);
	if (sprite) {
		sprite->action = ACT_LOOP;
		sprite->desc = 0;
		sprite->data = 0x062A;
	}

	/* Bone Animation */
	sprite = makeSprite(ST_ANIM, 0xE0, 0x00, 44, 184);
	if (sprite) {
		sprite->action = ACT_LOOP;
		sprite->desc = 0;
		sprite->data = 0x8804;
	}

	drawSpriteList();
	swapSpriteBuffer();	

	gameState = GS_FADE_IN;
	nextState = GS_HELP_ACTIVE;
}

void helpExit() {
	gameState = GS_FADE_OUT;
	nextState = GS_TITLE_LOAD;
}

void helpMove() {
	helpVolumeSelect ^= 1;
	helpVolumeUpdate();

	inputDelay = 2 * INPUT_DELAY;
}

#define MAX_ATT 9
#define MIN_ATT 0
void helpChange(delta)
	int delta;
{
	u_char changed = 0;
	switch (helpVolumeSelect) {
		case 0: /* Music */
			if (delta > 0 && gameSave.musicAttenuation < MAX_ATT) { gameSave.musicAttenuation++; changed = 1; }
			if (delta < 0 && gameSave.musicAttenuation > MIN_ATT) { gameSave.musicAttenuation--; changed = 1; }
		break;
		case 1: /* Sfx */
			if (delta > 0 && gameSave.sfxAttenuation < MAX_ATT) { gameSave.sfxAttenuation++; changed = 1; }
			if (delta < 0 && gameSave.sfxAttenuation > MIN_ATT) { gameSave.sfxAttenuation--; changed = 1; }
		break;
	}
	if (changed) {
		setGameAttenuation();
		helpVolumeUpdate();
		if (allowSave) saveGame();
		playSfx(SFX_PLHIT);
	}
	else {
		playSfx(SFX_MZFAIL);
	}

	inputDelay = 2 * INPUT_DELAY;
}

void helpLoop() {
	u_short input = readInput1();

	if (inputDelay) { inputDelay--; }
	else {
		if      (input & I_BUTTON1) helpExit();
		else if (input & I_RIGHT)   helpChange(-1);
		else if (input & I_LEFT)    helpChange( 1);
		else if (input & I_UP)      helpMove();
		else if (input & I_DOWN)    helpMove();
	}

	clearSprites();
	stepSprites();
	drawSpriteList();
	swapSpriteBuffer();
}
#define FINAL_SCORE_X 164
#define FINAL_SCORE_Y 167

#define FINAL_CHAR_X 164
#define FINAL_CHAR_Y (167 + 19)
#define NAME_CHAR_COUNT 42

char charIndex;

Score finalScore = {0, 0, 0, 0, 0, 0};



void drawWinChar(tile, x, y)
	u_char tile;
	short x, y;
{
	clearRect(spActive->videoBuffer, x, y, 8, 8, 0);
	copyRect(
		spriteSheet + HUD_FONT + ((tile & 0x20) << 6) + ((tile & 0x1f) << 3),
		spActive->videoBuffer, 
		x, y, 8, 8, 256
	);
}

void winMove(delta)
	char delta;
{
	char newIndex = charIndex + delta;
	if (newIndex < 0 || newIndex > 3) return;

	if (charIndex < 3) {
		drawWinChar(0x3F, FINAL_CHAR_X + 8 * charIndex, FINAL_CHAR_Y - 8);
		drawWinChar(0x3F, FINAL_CHAR_X + 8 * charIndex, FINAL_CHAR_Y + 8);
	}
	else {
		drawWinChar(0x3F, FINAL_CHAR_X + 8 * 4, FINAL_CHAR_Y);
	}

	charIndex = newIndex;

	if (charIndex < 3) {
		drawWinChar(0x2D, FINAL_CHAR_X + 8 * charIndex, FINAL_CHAR_Y - 8);
		drawWinChar(0x2C, FINAL_CHAR_X + 8 * charIndex, FINAL_CHAR_Y + 8);
	}
	else {
		drawWinChar(0x2F, FINAL_CHAR_X + 8 * 4, FINAL_CHAR_Y);
	}
}

void winChar(delta)
	char delta;
{
	switch (charIndex) {
		case 0:
			finalScore.name_0 = (finalScore.name_0 + NAME_CHAR_COUNT + delta) % NAME_CHAR_COUNT;
			drawWinChar(finalScore.name_0, FINAL_CHAR_X + 0 * 8, FINAL_CHAR_Y);
		break;
		case 1:
			finalScore.name_1 = (finalScore.name_1 + NAME_CHAR_COUNT + delta) % NAME_CHAR_COUNT;
			drawWinChar(finalScore.name_1, FINAL_CHAR_X + 1 * 8, FINAL_CHAR_Y);
		break;
		case 2:
			finalScore.name_2 = (finalScore.name_2 + NAME_CHAR_COUNT + delta) % NAME_CHAR_COUNT;
			drawWinChar(finalScore.name_2, FINAL_CHAR_X + 2 * 8, FINAL_CHAR_Y);
		break;
	}
}

void winLoad() {
	stopMusic();
	safeRead(readScore);
	playMusic(0);

	showPrimaryImageBuffer();

	initSpriteList();
	clearSpriteList();
	clearSpriteBuffers();

	/* Draw Timer and HUD, register score */
	finalScore.time = gameTime > MAX_TIME ? MAX_TIME : gameTime;

	drawTimerTime(gameTime);
	copyRect(timerBuffer, ipPrim->videoBuffer, FINAL_SCORE_X, FINAL_SCORE_Y, TIMER_WIDTH, TIMER_HEIGHT, TIMER_WIDTH);

	if (playerState.LevelNumber >= LEVEL_DEMO) {
		updateHudDemo();

		finalScore.bomb = playerState.BombMax;
		finalScore.fire = playerState.BombSize;

		if (playerState.BombType == ACT_FIRE) {
			finalScore.fire |= 0x08; /* Set highest bit */
		}

		copyRect(hudBuffer, ipPrim->videoBuffer, FINAL_SCORE_X + TIMER_WIDTH + 8, FINAL_SCORE_Y, HUD_WIDTH, HUD_HEIGHT, HUD_WIDTH);
	}
	else {
		updateHudGame();
		/* Split coin state over 2 variables */
		finalScore.bomb = playerState.Coins >> 4;
		finalScore.fire = playerState.Coins & 0x0F;
		copyRect(hudBuffer, ipPrim->videoBuffer, FINAL_SCORE_X + TIMER_WIDTH + 8, FINAL_SCORE_Y, HUD_WIDTH, HUD_HEIGHT, HUD_WIDTH);
	}

	charIndex = 0;
	inputDelay = INPUT_DELAY;
	
	drawWinChar(finalScore.name_0, FINAL_CHAR_X + 0 * 8, FINAL_CHAR_Y);
	drawWinChar(finalScore.name_1, FINAL_CHAR_X + 1 * 8, FINAL_CHAR_Y);
	drawWinChar(finalScore.name_2, FINAL_CHAR_X + 2 * 8, FINAL_CHAR_Y);
	
	drawWinChar( 4, FINAL_CHAR_X + 5 * 8, FINAL_CHAR_Y);
	drawWinChar(23, FINAL_CHAR_X + 6 * 8, FINAL_CHAR_Y);
	drawWinChar( 8, FINAL_CHAR_X + 7 * 8, FINAL_CHAR_Y);
	drawWinChar(19, FINAL_CHAR_X + 8 * 8, FINAL_CHAR_Y);

	winMove(0);

	gameState = GS_FADE_IN;
	nextState = GS_WIN_ACTIVE;
}

void saveScore(scoreList)
	Score *scoreList;
{
	int pos = 0, move;
	while (pos < (SCORE_COUNT - 1)) {
		if (scoreList[pos].time == 0 || scoreList[pos].time > finalScore.time) break;
		pos++;
	}

	for (move = SCORE_COUNT - 1; move > pos; move--) {
		scoreList[move] = scoreList[move - 1];
	}
	scoreList[pos] = finalScore;

	if (allowSave) saveGame();
}

void winExit() {
	if (charIndex == 3) {
		if (playerState.LevelNumber >= LEVEL_DEMO) {
			saveScore(gameSave.demoScore);
		}
		else if (playerState.Coins == 0x7F && playerState.BombMax == 2 && playerState.BombSize == 2) {
			saveScore(gameSave.fullScore);
		}
		else {
			saveScore(gameSave.gameScore);
		}

		gameState = GS_FADE_OUT;
		nextState = GS_TITLE_LOAD;
	}
}

void winLoop() {
	u_short input = readInput1();

	if (inputDelay) { inputDelay--; }
	else {
		if      (input & I_BUTTON1) winExit();
		else if (input & I_RIGHT) winMove( 1);
		else if (input & I_LEFT)  winMove(-1);
		else if (input & I_UP)    winChar( 1);
		else if (input & I_DOWN)  winChar(-1);

		inputDelay = INPUT_DELAY;
	}
}
