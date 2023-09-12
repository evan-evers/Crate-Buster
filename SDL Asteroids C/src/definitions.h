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

//Utility macros
#define MAX_STRING_LENGTH 128	//max length for any string in the game. maybe a bit low but this isn't exactly a text-heavy game
#define MAX_LABEL_LENGTH 64	//used for identifying label strings in structs
#define MAX_NAME_LENGTH 24	//max length of a name for a highscore
#define MAX_INPUT_LENGTH 16	//length of inputText in an InputManager object

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
#define NUM_BACKGROUND_STARS_L1 50
#define NUM_BACKGROUND_STARS_L2 50
#define NUM_BACKGROUND_STARS_L3 50
#define NUM_HIGHSCORES 5	//the number of highscores that will be saved

#define MAX_KEYBOARD_KEYS 150
#define GAMEPAD_AXIS_MAX 32767
#define GAMEPAD_AXIS_MIN -32768
#define MAX_MOUSE_BUTTONS 50

//Audio macros
#define MAX_SOUND_CHANNELS 8	//the maximum number of sound channels, and the maximum number of sounds that can be played simultaneously
#define PAN_CENTER 127			//pass into playSound or playSoundIsolated to keep sound centered

#endif