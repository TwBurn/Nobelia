#include <csd.h>
#include <sysio.h>
#include <ucm.h>
#include <cdfm.h>
#include <memory.h>
#include <stdio.h>
#include <setsys.h>

#include "cdio.h"

int intHandler(sigCode)
	int sigCode;
{
	handleVideoSignal(sigCode);
	handleAudioSignal(sigCode);
}

void initSystem()
{
	intercept(intHandler);
	initVideo();
	initGraphics();
	initInput();
	initAudio();
	initGame();
	initTitle();
}

void closeSystem()
{
	closeVideo();
	closeInput();
	closeAudio();
}

void runMain() {
	
	if (readIntro() < 0) return;

	startAudio();
	runIntro();

	if (readGameData() < 0) return;
	
	gameLoop();
}

int main(argc, argv)
	int argc;
	char* argv[];
{
	initSystem();
	runMain();	
	closeSystem();
	exit(0);
}
