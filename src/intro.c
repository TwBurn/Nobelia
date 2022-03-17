#include <csd.h>
#include <sysio.h>
#include <stdio.h>
#include <ucm.h>
#include <events.h>

#include "graphics.h"
#include "input.h"
#include "audio.h"

#define COL_COUNT 96
#define MOD_FACTOR (128 * SCREEN_HEIGHT)

u_short colPos[] = {31393,9433,20544,1579,34126,22043,10363,26943,11139,26995,35054,22218,28233,4311,15521,26741,5751,8426,7401,8138,6571,24173,13065,8309,23521,4307,21858,18296,20817,19831,3712,1400,23516,33318,17142,22656,1234,33216,18587,25287,11486,11948,2913,24243,29028,5028,550,1016,13689,7015,10113,9449,32676,29907,4615,31290,29930,16993,7149,35452,5056,24807,14176,34086,20239,929,23570,23294,2224,8833,18692,21480,17412,33978,28549,14769,23266,28215,3079,3405,21572,12308,25359,9595,4333,16822,31448,35,26930,31582,16927,21326,8662,18114,35140,3844};
u_short colStep[] = {115,102,57,113,87,120,110,68,108,65,96,101,109,119,76,55,59,99,93,67,55,117,89,69,90,98,104,73,71,78,95,71,50,78,117,100,76,101,98,84,57,65,64,88,60,100,112,88,63,79,81,51,109,109,97,80,109,110,105,92,116,88,85,109,63,107,64,120,108,104,76,78,97,51,80,110,63,116,57,76,114,74,80,89,53,123,57,107,65,94,115,110,73,64,114,64};
/*{40,61,66,80,51,45,103,102,94,95,52,50,119,79,118,83,120,42,102,75,104,73,72,71,91,51,117,82,53,41,74,108,62,96,100,100,90,118,120,34,96,54,58,104,127,64,77,52,88,71,101,103,75,116,99,56,67,113,79,120,106,103,90,118,97,53,51,34,93,57,57,37,38,38,92,86,88,80,32,94,73,96,98,73,41,81,72,122,102,77,105,62,85,126,40,34};*/


#define INTRO_DURATION 500
void runIntro()
{
	u_short input;
	u_short introTime = INTRO_DURATION;

	register u_int x, y, p, v;

	register u_int* drawBuffer = (u_int*)ipPrim->videoBuffer;
	register u_int* readBuffer = (u_int*)ipSec->videoBuffer;

	setIcf(ICF_MIN, ICF_MIN);
	
	playMusic(6);
	decodeIntro();
	showPrimaryImageBuffer();

	dc_ssig(videoPath, SIG_BLANK, 0);
	p = 0x00000101;
	v = 0x00000000;
	while (introTime) {
		if ((frameTick & 7) == 0) p = (p & 0x03030303) + 0x11111111;
		for (x = 0; x < COL_COUNT; x++) {
			p = (p & 0x03030303) + 0x11111111;
			colPos[x] = (colPos[x] + 2*colStep[x]) % MOD_FACTOR;
			y = colPos[x] >> 7;
			if (y == 0) colStep[x] = rand() % 96 + 32;
			y = (96 * y + x);
			drawBuffer[y] = readBuffer[y]; y = (y + 96) % 26880;
			drawBuffer[y] = readBuffer[y]; y = (y + 96) % 26880;
			drawBuffer[y] = p; y = (y + 96) % 26880;
			drawBuffer[y] = p; y = (y + 96) % 26880;p = (p & 0x03030303) + 0x11111111;
			drawBuffer[y] = p; y = (y + 96) % 26880;
			drawBuffer[y] = p;
		}


		input = readInput1();

		if (introTime > ICF_MAX) {
			if (input & I_BUTTON_ANY) introTime = ICF_MAX;
			if (curIcfA != ICF_MAX) setIcf(curIcfA + 1, curIcfA + 1);
		}
		else {
			setIcf(introTime, introTime);
			setAttenuation(0x80 - 2 * introTime, 0x80 - 2 * introTime);
		}
		introTime--;
		while (!frameDone) {}; /* Wait for SIG_BLANK */
		frameDone = 0;
		dc_ssig(videoPath, SIG_BLANK, 0);
	}
	setIcf(ICF_MIN, ICF_MIN);
	stopMusic();
}