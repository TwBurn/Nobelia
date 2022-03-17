#ifndef __CDIO_H__
#define	__CDIO_H__

#define RETRY_COUNT 3

extern int titleLoaded;

int readIntro();
int readGameData();
int readScore();
int readTitle();
int readLevels();

void safeRead(readFunc);

#endif