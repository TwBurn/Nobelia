#ifndef __GAME_H__
#define	__GAME_H__

#ifndef NULL
#define NULL 0
#endif

#define GS_QUIT_GAME    0x0000
#define GS_TITLE_LOAD   0x0001
#define GS_TITLE_ACTIVE 0x0002
#define GS_TITLE_OUT    0x0003
#define GS_TITLE_PICK   0x0004
#define GS_HELP_LOAD    0x0005
#define GS_HELP_ACTIVE  0x0006
#define GS_SCORE_LOAD   0x0007
#define GS_SCORE_ACTIVE 0x0008
#define GS_DISC_ERROR   0x00EE
#define GS_LEVEL_LOAD   0x001F
#define GS_GAME_START   0x0010
#define GS_GAME_ACTIVE  0x0020
#define GS_GAME_PAUSE   0x0030
#define GS_GAME_RESUME  0x0031
#define GS_WIN_LOAD     0x0040
#define GS_WIN_ACTIVE   0x0041
#define GS_FADE_IN      0x00F1
#define GS_FADE_OUT     0x00F0

#define ST_NONE    0x00
#define ST_PLAYER  0x01
#define ST_BOMB    0x02
#define ST_FIRE    0x03
#define ST_MAGIC   0x04
#define ST_CLOAK   0x05
#define ST_COIN    0x0C
#define ST_SWITCH  0x10
#define ST_TRIGGER 0x11
#define ST_CHEST   0x20
#define ST_TRAP    0x30
#define ST_BURNER  0x31
#define ST_CANNON  0x40
#define ST_TORCH   0x41
#define ST_PEDESTL 0x42
#define ST_CNBALL  0x48
#define ST_BLOOM   0x50
#define ST_ENEMY   0x80
#define ST_BONE    0x90
#define ST_ANIM    0xA0
#define ST_SCRIPT  0xC0


#define ET_SKELET  0x20
#define ET_GHOST   0x30
#define ET_BLOB    0x40
#define ET_SPIDER  0x50
#define ET_BAT     0x60

/* Attack Types */
#define AT_NONE    0x00  /* No Attack */
#define AT_PROJ    0x01  /* Projectile */
#define AT_FIRE    0x02  /* Fire */

/* Action 
	0	No draw
	1-7 Draw Priority Medium
	8-B Draw Priority High
	C-F Draw Priority Low
*/
#define ACT_HIDDEN 0x0000 
#define ACT_IDLE   0x1000
#define ACT_ACTIVE 0x2000
#define ACT_MOVE   0x3000
#define ACT_FIRE   0x4000
#define ACT_LOOP   0x5000
#define ACT_USER   0x6000
#define ACT_BURN   0xB000
#define ACT_HIT    0xC000
#define ACT_DEAD   0xD000
#define ACT_EXIT   0xE000

#define DIR_NONE   0x0000
#define DIR_ALL    0x0F00
#define DIR_LEFT   0x0100
#define DIR_RIGHT  0x0200
#define DIR_UP     0x0400
#define DIR_DOWN   0x0800

#define LEVEL_COUNT 37
#define LEVEL_DEMO  33
#define LEVEL_CONTINUE 1

#define SPRITE_COUNT   36
#define EXIT_COUNT 8

#define FADE_SPEED 2

#define dir_rev(dir)  (dir==DIR_LEFT?DIR_RIGHT:(dir==DIR_RIGHT?DIR_LEFT:(dir==DIR_UP?DIR_DOWN:(dir==DIR_DOWN?DIR_UP:DIR_NONE))))
#define dir_next(dir) (dir==DIR_LEFT?DIR_UP:(dir==DIR_RIGHT?DIR_DOWN:(dir==DIR_UP?DIR_RIGHT:(dir==DIR_DOWN?DIR_LEFT:DIR_NONE))))
#define dir_prev(dir) (dir==DIR_LEFT?DIR_DOWN:(dir==DIR_RIGHT?DIR_UP:(dir==DIR_UP?DIR_LEFT:(dir==DIR_DOWN?DIR_RIGHT:DIR_NONE))))
#define dir_frame(dir) (dir==DIR_LEFT?0x04:(dir==DIR_RIGHT?0x08:(dir==DIR_UP?0x0C:(dir==DIR_DOWN?0x00:0))))
#define dir_dx(dir) (dir==DIR_LEFT?-16:(dir==DIR_RIGHT?16:0))
#define dir_dy(dir) (dir==DIR_UP?-16:(dir==DIR_DOWN?16:0))
#define dir_tile(dir) (dir==DIR_LEFT?-1:(dir==DIR_RIGHT?1:(dir==DIR_UP?-23:(dir==DIR_DOWN?23:0))))
#define pix_dx(dir) (dir==DIR_LEFT?-1:(dir==DIR_RIGHT?1:0))
#define pix_dy(dir) (dir==DIR_UP?-1:(dir==DIR_DOWN?1:0))

#define SCORE_COUNT 8
#define MAX_TIME (10 * 3600 * 50 - 1)

typedef struct Score {
	
	u_int bomb:   4;
	u_int time:   22;
	u_int name_0: 6;
	u_int name_1: 6;
	u_int name_2: 6;
	u_int fire:   4;
} Score;

typedef struct SaveFile {
	u_int saveState;
	u_int sfxAttenuation: 4;
	u_int musicAttenuation: 4;
	u_int elapsedTime: 24;
	Score gameScore[SCORE_COUNT];
	Score fullScore[SCORE_COUNT];
	Score demoScore[SCORE_COUNT];
} SaveFile;

typedef struct Sprite
{
	u_char type;
	u_char tile;
	u_char frame;
	u_char desc;
	u_short action;
	u_short data;
	int     word;
	short x;
	short y;
} Sprite;

typedef struct LevelTile {
	u_char move;
	u_char top;
	u_char middle;
	u_char bottom;
} LevelTile;

typedef struct LevelExit {
	u_short tile;
	u_char  next;
	u_char  direction;
	u_char  type;
	u_char  spawnDir;
	u_short spawnTile;
} LevelExit;

typedef struct Level {
	u_char name[7];
	u_char musicChannel;
	u_int  border;
	u_int  background;
	u_char reserved[12];
	LevelTile tileMap[23 * 15];
	Sprite object[SPRITE_COUNT];
	LevelExit exit[EXIT_COUNT];
} Level;

typedef struct PlayerState {
	u_char  BombMax;
	u_char  BombCur;
	u_char  BombSize;
	u_short BombType; /* ACT_FIRE or ACT_USER */
	u_char  LevelNumber;
	u_short SpawnDir;
	u_short SpawnTile;
	u_char  PowerUp;
	u_char  Coins;
	u_int   ChestState;
} PlayerState;

extern PlayerState playerState;

extern SaveFile gameSave;

extern int gameState, nextState;
extern u_int frameTick, gameTime, templeValue;
extern u_char allowSave;

extern Level gameLevel;
extern Sprite *player;

u_int rand();

Sprite* makeSprite(type, tile, frame, x, y);
Sprite* findSprite(x, y);
Sprite* findFire(x, y);
Sprite* attackSprite(x, y, dir, attackType, data);

void invokeScript(scriptId, sprite, data);

void drawLevelTile(index);
void updateLevelTile(index, top, move, anim);

#endif