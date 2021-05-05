#include <csd.h>
#include <sysio.h>
#include <ucm.h>
#include <cdfm.h>
#include <memory.h>
#include <stdio.h>
#include <setsys.h>
#include "build.h"
#include "graphics.h"

int intHandler(sigCode)
	int sigCode;
{
	handleGameSignal(sigCode);
	handleAudioSignal(sigCode);
}

void initSystem()
{
	intercept(intHandler);
	initVideo();
	initGraphics();
	initInput();
	initAudio();
}

void closeSystem()
{
	closeVideo();
	closeInput();
	closeAudio();
}

void loadData()
{
	int file = open("DATA.RTF", READ_);
	loadSpriteSheet(file);
	loadImage(file, ipDraw->videoBuffer);
	loadSfx(file);
	close(file);
}


int main(argc, argv)
	int argc;
	char* argv[];
{
	int curInput, oldInput;
	printf(BUILD_VERSION);
	initSystem();
	gameStart();

	closeSystem();
	return 0;
}
