#ifndef __INPUT_H__
#define	__INPUT_H__

#define I_BUTTON1 0x01
#define I_BUTTON2 0x02
#define I_BUTTON3 0x04
#define I_BUTTON_ANY (I_BUTTON1 | I_BUTTON2 | I_BUTTON3)
#define I_LEFT    0x10
#define I_RIGHT   0x20
#define I_UP      0x40
#define I_DOWN    0x80
#define I_SIGNAL1 0x0D00
#define I_SIGNAL2 0x0D01

unsigned short readInput1();
unsigned short readInput2();
unsigned int   readInput();

#endif