#ifndef STRUCTS_H
#define STRUCTS_H

/*
* The game's structs.
*/

#include "draw.h"

//Stores pointers to the specific logic and drawing functions being used at a given moment
typedef struct {
	void (*logic)(void);
	void (*draw)(void);
} Delegate;

//Stores various important things related to running the application
typedef struct {
	SDL_Renderer*	renderer;	//App's main (and only) renderer
	SDL_Window*		window;		//App's main (and only) window
	Delegate		delegate;	//logic and draw function pointers
	bool			quit;		//controls when to exit the main loop and quit the app.
	SpriteAtlas		*fontsAndUI;			//A sprite sheet of all fonts and UI
	SpriteAtlas		*gameplaySprites;		//A sprite sheet of sprites used in gameplay
	float			windowPixelRatioW;		//Width of the game screen space in pixels over actual width of window in pixels
	float			windowPixelRatioH;		//Height of the game screen space in pixels over actual height of window in pixels
	int				windowPaddingW;			//The letterboxing of the window in pixels (combined width of both edges)
	int				windowPaddingH;			//The letterboxing of the window in pixels (combined height of both edges)
} App;

//Stores mouse info
typedef struct {
	int x;
	int y;
	int buttons[MAX_MOUSE_BUTTONS];
	int wheel;
} Mouse;

//enum for last controller detected
typedef enum {
	LCT_KEYBOARD_AND_MOUSE,
	LCT_JOYPAD
} LastControllerType;

//Stores input
typedef struct {
	//raw input data
	SDL_Joystick *joypad;	//the joypad to be used in-game
	Mouse mouse; //Holds mouse position
	bool	keyboard[MAX_KEYBOARD_KEYS];	//holds raw keyboard input
	bool	joypadButtons[MAX_JOYPAD_BUTTONS];	//holds raw joypad button input
	int		joypadAxes[MAX_JOYPAD_BUTTONS];	//holds raw joypad axis input

	//gameplay control interface
	//only these variables should be used for checking for inputs outside of handleInput
	float lr;	//hold left-right directional input
	float ud;	//hold up-down directional input
	int deadzone;	//joystick deadzone
	bool fire;	//fire/confirm button
	bool back;	//go back button
	bool pause;	//pause button
	LastControllerType lastControllerType;	//keeps track of the controller type most recently used (keyboard and mouse or joypad)
} InputManager;

#endif