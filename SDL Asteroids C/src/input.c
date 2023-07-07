#include "common.h"

#include "input.h"

extern App app;
extern InputManager input;

//checking for a change

//window was resized; update certain constants
static void handleWindowResize(const SDL_WindowEvent* event) {
	//data1 and data2 seem to be the window's width and height
	if (event->data1 != 0 && event->data2 != 0) {
		//find letterboxing
		app.windowPaddingW = 0;
		app.windowPaddingH = 0;

		//horizontal letterboxing
		if (((float)event->data1 / (float)event->data2) > ASPECT_RATIO)
			app.windowPaddingW = event->data1 - ASPECT_RATIO * (double)event->data2;
		
		//vertical letterboxing
		if (((float)event->data2 / (float)event->data1) > ASPECT_RATIO_INV)
			app.windowPaddingH = event->data2 - ASPECT_RATIO_INV * (double)event->data1;

		//set ratios only according to the non-letterbox parts of the screen	
		app.windowPixelRatioW = (double)SCREEN_WIDTH / (double)(event->data1 - app.windowPaddingW);
		app.windowPixelRatioH = (double)SCREEN_HEIGHT / (double)(event->data2 - app.windowPaddingH);
	}
}

//keyboard control handling
static void doKeyUp(const SDL_KeyboardEvent* event)
{
	if (event->repeat == 0 && event->keysym.scancode >= 0 && event->keysym.scancode < MAX_KEYBOARD_KEYS) {
		input.keyboard[event->keysym.scancode] = false;
	}
}

static void doKeyDown(const SDL_KeyboardEvent* event)
{
	if (event->repeat == 0 && event->keysym.scancode >= 0 && event->keysym.scancode < MAX_KEYBOARD_KEYS) {
		input.keyboard[event->keysym.scancode] = true;

		//update most recently used controller (keyboard and mouse or joypad)
		input.lastControllerType = LCT_KEYBOARD_AND_MOUSE;
	}
}

//initializes joypad and handles controller connection and disconnection
//not static b/c init.c uses this too
void initJoypad(void) {
	//reset joypad
	if (input.joypad != NULL) {
		SDL_JoystickClose(input.joypad);
		//Fun fact: if input.joypad isn't NULLed, SDL_NumJoysticks throws an exception in visual studio!
		//This is why NULLing pointers when they point to deallocated memory is important.
		input.joypad = NULL;
	}

	//number of joypads
	int n;

	//find out how many joypads there are
	if (SDL_NumJoysticks() != 0) {
		n = SDL_NumJoysticks();
	}
	else {
		//no joypads found; none connected or some other problem
		printf("No joypads found: %s\n", SDL_GetError());
		return;
	}

	printf("%d joypad(s) available.\n", n);

	//go through the available joypads and use the first one found
	for (int i = 0; i < n; i++) {
		input.joypad = SDL_JoystickOpen(i);

		//check for NULL in case the user disconnected the joypad
		if (input.joypad != NULL) {
			printf("Joypad name: %s\n", SDL_JoystickNameForIndex(i));
			printf("Joypad axes: %d\n", SDL_JoystickNumAxes(input.joypad));
			printf("Joypad buttons: %d\n", SDL_JoystickNumButtons(input.joypad));

			return;
		}
	}
}

//joypad control handling
static void doButtonUp(const SDL_JoyButtonEvent* event)
{
	if (event->state == SDL_RELEASED && event->button < MAX_JOYPAD_BUTTONS) {
		input.joypadButtons[event->button] = false;
	}
}

static void doButtonDown(const SDL_JoyButtonEvent* event)
{
	if (event->state == SDL_PRESSED && event->button < MAX_JOYPAD_BUTTONS) {
		input.joypadButtons[event->button] = true;

		//update most recently used controller (keyboard and mouse or joypad)
		input.lastControllerType = LCT_JOYPAD;
	}
}

static void doJoyAxis(const SDL_JoyAxisEvent* event)
{
	if (event->axis < MAX_JOYPAD_AXES) {
		input.joypadAxes[event->axis] = event->value;

		//update most recently used controller (keyboard and mouse or joypad)
		input.lastControllerType = LCT_JOYPAD;
	}
}

//mouse control handling (some is also done in handleInput)
static void doMouseButtonUp(const SDL_MouseButtonEvent* event) {
	input.mouse.buttons[event->button] = false;
}

static void doMouseButtonDown(const SDL_MouseButtonEvent* event) {
	input.mouse.buttons[event->button] = true;

	//update most recently used controller (keyboard and mouse or joypad)
	input.lastControllerType = LCT_KEYBOARD_AND_MOUSE;
}

static void doGameplayInput(void) {
	//initialize all controls in unpressed/neutral positions
	input.lr = 0;
	input.ud = 0;
	input.fire = false;
	input.back = false;
	input.pause = false;

	//directional input
	if (input.joypad != NULL && ((abs(input.joypadAxes[0]) > input.deadzone || abs(input.joypadAxes[1]) > input.deadzone))) {
		//joystick input
		input.lr = input.joypadAxes[0];
		input.ud = input.joypadAxes[1];
	}
	else {
		//do keyboard input if there's no controller plugged in or if there's no joystick input
		
		//find direction
		input.lr = (input.keyboard[SDL_SCANCODE_D] - input.keyboard[SDL_SCANCODE_A]);
		input.ud = (input.keyboard[SDL_SCANCODE_S] - input.keyboard[SDL_SCANCODE_W]);

		//for compatability with the joysticks, direction along an axis will be either 0, JOYPAD_AXIS_MAX, or JOYPAD_AXIS_MIN
		if (input.lr > 0) {
			input.lr = JOYPAD_AXIS_MAX;
		}
		if (input.lr < 0) {
			input.lr = JOYPAD_AXIS_MIN;
		}
		if (input.ud > 0) {
			input.ud = JOYPAD_AXIS_MAX;
		}
		if (input.ud < 0) {
			input.ud = JOYPAD_AXIS_MIN;
		}
	}

	//normalize directions
	//technically off by ~.00001 when in a negative direction but who cares
	if (abs(input.lr) > input.deadzone || abs(input.ud > input.deadzone)) {
		input.lr /= JOYPAD_AXIS_MAX;
		input.ud /= JOYPAD_AXIS_MAX;
	}

	//buttons

	//left click or r1 to fire/confirm
	if (input.mouse.buttons[SDL_BUTTON_LEFT] || input.joypadButtons[5]) {
		input.fire = true;
	}

	//right click or circle to go back
	if (input.mouse.buttons[SDL_BUTTON_RIGHT] || input.joypadButtons[2]) {
		input.back = true;
	}

	//esc or options to pause
	if (input.keyboard[SDL_SCANCODE_ESCAPE] || input.joypadButtons[2]) {
		input.pause = true;
	}
}

void handleInput(void)
{
	SDL_Event event;

	//handle most events
	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
		case SDL_QUIT:
			app.quit = true;
			break;

		case SDL_WINDOWEVENT:
			if (event.window.event == SDL_WINDOWEVENT_RESIZED || event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
				handleWindowResize(&event.window);
			break;

		case SDL_KEYUP:
			doKeyUp(&event.key);
			break;

		case SDL_KEYDOWN:
			doKeyDown(&event.key);
			break;

		case SDL_JOYDEVICEADDED:
		case SDL_JOYDEVICEREMOVED:
			initJoypad();
			break;

		case SDL_JOYBUTTONUP:
			doButtonUp(&event.jbutton);
			break;

		case SDL_JOYBUTTONDOWN:
			doButtonDown(&event.jbutton);
			break;

		case SDL_JOYAXISMOTION:
			doJoyAxis(&event.jaxis);
			break;

		case SDL_MOUSEBUTTONUP:
			doMouseButtonUp(&event.button);
			break;

		case SDL_MOUSEBUTTONDOWN:
			doMouseButtonDown(&event.button);
			break;

		default:
			break;
		}
	}

	//handle scroll wheel
	if (event.type == SDL_MOUSEWHEEL) {
		input.mouse.wheel = event.wheel.y;

		//determine most recently used controller (keyboard and mouse or joypad)
		input.lastControllerType = LCT_KEYBOARD_AND_MOUSE;
	}
	else {
		input.mouse.wheel = 0;
	}

	//get mouse position
	SDL_GetMouseState(&input.mouse.x, &input.mouse.y);

	//adjust mouse position according to how the screen's been stretched (letterboxing taken into account by the windowPadding vars)
	input.mouse.x = ((float)input.mouse.x - (float)app.windowPaddingW * 0.5) * app.windowPixelRatioW;
	input.mouse.y = ((float)input.mouse.y - (float)app.windowPaddingH * 0.5) * app.windowPixelRatioH;

	//package raw input data into gameplay input interface
	doGameplayInput();
}
