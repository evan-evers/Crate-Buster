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
	bool			debug;					//Turns on hitbox viewing (and maybe other stuff down the line)
} App;

//Stores mouse info
typedef struct {
	int x;
	int y;
	uint8_t buttons[MAX_MOUSE_BUTTONS];
	int wheel;
} Mouse;

//enum for last controller detected
typedef enum {
	LCT_KEYBOARD_AND_MOUSE,
	LCT_GAMEPAD
} LastControllerType;

//bit flag enum for input flags, which store specific info about inputs
//the way the input system works is by storing these flags in uint_8 arrays in an InputManager
//Some flags should be combined, while others should not
//For instance, IS_HELD and IS_PRESSED should be combined on the first update that an input is pressed
//But IS_HELD should never be combined with IS_RELEASED
typedef enum {
	IS_NONE = 0,	//no relevant input information
	IS_HELD = 1,	//input is being held down
	IS_PRESSED = 2,	//input was pressed on this update, but was not pressed on the update before
	IS_RELEASED = 4	//input was released on this update, but was held down on the update before
} InputState;

//Stores input
typedef struct {
	//raw input data
	SDL_GameController *gamepad;	//the gamepad to be used in-game
	Mouse mouse; //Holds mouse position
	uint8_t	keyboard[MAX_KEYBOARD_KEYS];	//holds raw keyboard input
	uint8_t	gamepadButtons[SDL_CONTROLLER_BUTTON_MAX];	//holds raw gamepad button input
	int		gamepadAxes[SDL_CONTROLLER_AXIS_MAX];	//holds raw gamepad axis inputs

	//gameplay control interface
	//only these variables should be used for checking for inputs outside of handleInput
	//vars corresponding to buttons are ints, rather than bools, to allow for input buffering
		//This also means you have to check if one of these variables is greater than zero to see if it's true, and 0 or less to see if it's false
		//Additionally, after checking a press, make sure to reset the buffer to 0.
	float leftLR;	//holds left-right directional input for left analog stick
	float leftUD;	//holds up-down directional input for left analog stick
	float rightLR;	//holds left-right directional input for right analog stick
	float rightUD;	//holds up-down directional input for right analog stick
	int deadzone;	//joystick deadzone
	int fire;	//fire/confirm button
	int firePressed;	//pressed variation
	int dash;	//dash/go back button
	int dashPressed;	//pressed variation
	int pause;	//pause button
	int pausePressed;	//pressed variation
	LastControllerType lastControllerType;	//keeps track of the controller type most recently used (keyboard and mouse or gamepad)
} InputManager;

#endif