#ifndef DEFINITIONS_H
#define DEFINITIONS_H

/*
* A set of macros used throughout the project.
* There shouldn't be any variables related to gameplay functionality here.
*/

//Utility functions
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define STRNCPY(dest, src, n) \
	strncpy(dest, src, n);    \
	dest[n - 1] = '\0'

//Game macros
#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 360
#define ASPECT_RATIO (float)16/9		//w/h
#define ASPECT_RATIO_INV (float)9/16	//h\w

#define SPRITE_ATLAS_CELL_W 16
#define SPRITE_ATLAS_CELL_H 16

#define FPS 60

#define MAX_KEYBOARD_KEYS 150
#define MAX_JOYPAD_BUTTONS 24
#define MAX_JOYPAD_AXES 8
#define JOYPAD_AXIS_MAX 32767
#define JOYPAD_AXIS_MIN -32768
#define MAX_MOUSE_BUTTONS 10

//Audio macros
#define MAX_SND_CHANNELS 8


#endif