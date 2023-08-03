#ifndef DEFINITIONS_H
#define DEFINITIONS_H

/*
* A set of macros used throughout the project.
* There shouldn't be any variables related to gameplay functionality here.
*/

//Utility functions
#define MIN(a, b) ((a < b) ? a : b)
#define MAX(a, b) ((a > b) ? a : b)
#define STRNCPY(dest, src, n) \
	strncpy(dest, src, n);    \
	dest[n - 1] = '\0'
#define REPEAT(n) \
	for(int i = 0; i < n; ++i)

//Game macros
#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 360
#define ASPECT_RATIO (float)16/9		//w/h
#define ASPECT_RATIO_INV (float)9/16	//h\w

#define SPRITE_ATLAS_CELL_W 16
#define SPRITE_ATLAS_CELL_H 16

#define FPS 60

//how far off of the screen to go before screenwrapping (in sprite width/height)
//0.75 is just an approximation of the square root of 2 (1.414 something) divided by 2
#define SCREENWRAP_MARGIN (float)0.75
//time when hitflash and powerup collection flash no longer get drawn
#define END_OF_FLASH 15
#define PLAYER_I_FRAMES_MAX 60
#define MAX_ENEMIES 10	//largest number of enemies that can be in the game at once
#define BULLET_OFFSET_PLAYER 20	//offset from the center of the player when a player bullet is created
#define BULLET_OFFSET_ENEMY 25	//offset from the center of an enemy when an enemy bullet is created
#define NUM_BACKGROUND_STARS_L1 25
#define NUM_BACKGROUND_STARS_L2 25
#define NUM_BACKGROUND_STARS_L3 25

#define MAX_KEYBOARD_KEYS 150
#define GAMEPAD_AXIS_MAX 32767
#define GAMEPAD_AXIS_MIN -32768
#define MAX_MOUSE_BUTTONS 50

//Audio macros
#define MAX_SND_CHANNELS 8


#endif