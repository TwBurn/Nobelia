#include "sys.h"
#include "input.h"

#define PT_CENTER 512
#define PT_FACT 3

int input1Path, input2Path;
u_short input1State, input2State;

int dx1, dx2;
int dy1, dy2;

void initInput()
{
	u_char *devName;

	devName = (u_char*)csd_devname(DT_PTR, 1);
	if (devName) {
		input1Path = open(devName, UPDAT_);
		free(devName);
	}

	devName = (u_char*)csd_devname(DT_PTR, 2);
	if (devName) {
		input2Path = open(devName, UPDAT_);
		free(devName);
	}

	readInput();
	readInput();
}

void closeInput()
{
	if (input1Path) {
		close(input1Path);
		input1Path = 0;
	}
	if (input2Path) {
		close(input2Path);
		input2Path = 0;
	}
}



u_short readInput1() {
	int x, y, buttons;
	int dx, dy;
	u_short inputState = 0;

	if (input1Path) {
		pt_coord(input1Path, &buttons, &x, &y);
		buttons &= 3;
		if (buttons == 3) {
			inputState |= I_BUTTON3;
		}
		else {
			inputState |= buttons;
		}
		
		x = (x - PT_CENTER) >> PT_FACT; /* Discard last bits to ignore very small movement */
		y = (y - PT_CENTER) >> PT_FACT;

		dx = dx1 + x;
		dy = dy1 + y;

		/* Handle coordinate wrap-around */
				
		if (dx < 0) inputState |= I_LEFT;
		if (dx > 0) inputState |= I_RIGHT;
		if (dy < 0) inputState |= I_UP;
		if (dy > 0) inputState |= I_DOWN;

		dx1 = x;
		dy1 = y;

		pt_pos(input1Path, PT_CENTER, PT_CENTER);

		return inputState;
	}
	else {
		return 0xFF;
	}
}

u_short readInput2() {
	int x, y, buttons;
	int dx, dy;
	u_short inputState = 0;

	if (input2Path) {
		pt_coord(input2Path, &buttons, &x, &y);
		buttons &= 3;
		if (buttons == 3) {
			inputState |= I_BUTTON3;
		}
		else {
			inputState |= buttons;
		}
		
		x = (x - PT_CENTER) >> PT_FACT; /* Discard last bits to ignore very small movement */
		y = (y - PT_CENTER) >> PT_FACT;

		dx = dx2 + x;
		dy = dy2 + y;

		/* Handle coordinate wrap-around */
				
		if (dx < 0) inputState |= I_LEFT;
		if (dx > 0) inputState |= I_RIGHT;
		if (dy < 0) inputState |= I_UP;
		if (dy > 0) inputState |= I_DOWN;

		dx2 = x;
		dy2 = y;

		return inputState;
	}
	else {
		return 0xFF;
	}
}

u_int readInput()
{
	u_int inputState = readInput2();
	inputState = (inputState << 8) | readInput1();
	return inputState;
}

void setInputSignals()
{
	if (input1Path) pt_ssig(input1Path, I_SIGNAL1);
	if (input2Path) pt_ssig(input2Path, I_SIGNAL2);
}

void handleInputSignal(signal)
	int signal;
{
	u_short curState;
	
	if (signal == I_SIGNAL1) {
		curState = readInput1();
		input1State = (input1State & 0xF0) | curState;
		pt_ssig(input1Path, signal);
	}
	else if (signal == I_SIGNAL2) {
		curState = readInput2();
		input2State = (input2State & 0xF0) | curState;
		pt_ssig(input2Path, signal);
	}
}
