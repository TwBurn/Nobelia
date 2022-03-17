#include <sysio.h>
#include <ucm.h>
#include <cdfm.h>
#include <stdio.h>

#include "cdio.h"
#include "graphics.h"

int titleLoaded = 0;

int readIntro() {
	int file = open("INTRO.RTF", READ_);
	int res = file;
	res |= readIntroData(file);
	return res;
}

int readGameData()
{
	int file = open("GAME.RTF", READ_);
	int res = file;

	res |= readTileSheet(file);
	res |= readSpriteSheet(file);
	res |= readInstructions(file);
	res |= readDiscError(file);
	res |= readSfx(file);

	close(file);
	return res;
}

int readTitle()
{
	int file, res;

	if (titleLoaded) {
		return 1;
	}
	else {
		file = open("TITLE.RTF", READ_);
		res = file;
		res |= readImage(file, ipPrim->videoBuffer);
		res |= readImage(file, ipSec->videoBuffer);
		close(file);

		if (res >= 0) {
			titleLoaded = 1;
		}
	}
	return res;
}

int readScore()
{
	int file = open("SCORE.RTF", READ_);
	int res = file;
	res |= readImage(file, ipPrim->videoBuffer);
	close(file);
	titleLoaded = 0;

	return res;
}

int readLevels()
{
	int file = open("WORLD.RTF", READ_);
	int res = file;
	res |= readLevelData(file);
	close(file);

	return res;
}

void safeRead(readFunc)
	int (*readFunc)();
{
	int retry = RETRY_COUNT;
	while (1) {	
		if (readFunc() > 0) return;
		if (retry-- == 0) {
			discError();
			retry = RETRY_COUNT;
		}
	}
}