#include "common.h"

#include "input.h"

extern App app;
extern InputManager input;

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

		//update most recently used controller (keyboard and mouse or gamepad)
		input.lastControllerType = LCT_KEYBOARD_AND_MOUSE;
	}
}

//initializes gamepad and handles controller connection and disconnection
//not static b/c init.c uses this too
void initGamepad(void) {
	//reset gamepad
	if (input.gamepad != NULL) {
		SDL_GameControllerClose(input.gamepad);
		//Fun fact: if input.gamepad isn't NULLed, SDL_NumJoysticks throws an exception in visual studio!
		//This is why NULLing pointers when they point to deallocated memory is important.
		input.gamepad = NULL;
	}

	//number of gamepads
	int n;

	//find out how many gamepads there are
	if (SDL_NumJoysticks() != 0) {
		n = SDL_NumJoysticks();
	}
	else {
		//no gamepads found; none connected or some other problem
		printf("No gamepads found: %s\n", SDL_GetError());
		return;
	}

	printf("%d gamepad(s) available.\n", n);

	//go through the available gamepads and use the first one found
	for (int i = 0; i < n; i++) {
		//only initialize joystick if it's a valid gamepad
		if(SDL_IsGameController(i))
			input.gamepad = SDL_GameControllerOpen(i);

		//check for NULL in case the user disconnected the gamepad
		if (input.gamepad != NULL) {
			printf("Gamepad name: %s\n", SDL_GameControllerName(input.gamepad));

			return;
		}
	}
}

//gamepad control handling
static void doGamepadButtonUp(const SDL_ControllerButtonEvent* event)
{
	if (event->state == SDL_RELEASED && event->button < SDL_CONTROLLER_BUTTON_MAX) {
		input.gamepadButtons[event->button] = false;
	}
}

static void doGamepadButtonDown(const SDL_ControllerButtonEvent* event)
{
	if (event->state == SDL_PRESSED && event->button < SDL_CONTROLLER_BUTTON_MAX) {
		input.gamepadButtons[event->button] = true;

		//update most recently used controller (keyboard and mouse or gamepad)
		input.lastControllerType = LCT_GAMEPAD;
	}
}

static void doGamepadAxis(const SDL_ControllerAxisEvent* event)
{
	if (event->axis < SDL_CONTROLLER_AXIS_MAX) {
		input.gamepadAxes[event->axis] = event->value;

		//NOTE: don't update input.lastControllerType here
		//because this gets called every frame (presumably b/c of stick drift)
	}
}

//mouse control handling (some is also done in handleInput)
static void doMouseButtonUp(const SDL_MouseButtonEvent* event) {
	input.mouse.buttons[event->button] = false;
}

static void doMouseButtonDown(const SDL_MouseButtonEvent* event) {
	input.mouse.buttons[event->button] = true;

	//update most recently used controller (keyboard and mouse or gamepad)
	input.lastControllerType = LCT_KEYBOARD_AND_MOUSE;
}

static void doMouseWheel(const SDL_MouseWheelEvent* event) {

}

static void doGameplayInput(void) {
	//initialize all controls in unpressed/neutral positions
	input.leftLR = 0;
	input.leftUD = 0;
	input.rightLR = 0;
	input.rightUD = 0;
	input.fire = false;
	input.dash = false;
	input.pause = false;

	//directional input
	if (input.gamepad != NULL && (abs(input.gamepadAxes[SDL_CONTROLLER_AXIS_LEFTX]) > input.deadzone || abs(input.gamepadAxes[SDL_CONTROLLER_AXIS_LEFTY]) > input.deadzone)) {
		//analog stick input
		input.leftLR = input.gamepadAxes[SDL_CONTROLLER_AXIS_LEFTX];
		input.leftUD = input.gamepadAxes[SDL_CONTROLLER_AXIS_LEFTY];
	}
	else {
		//do keyboard/d-pad input if there's no controller plugged in or if there's no analog stick input
		
		//find direction
		if (input.lastControllerType == LCT_KEYBOARD_AND_MOUSE) {
			input.leftLR = (input.keyboard[SDL_SCANCODE_D] - input.keyboard[SDL_SCANCODE_A]);
			input.leftUD = (input.keyboard[SDL_SCANCODE_S] - input.keyboard[SDL_SCANCODE_W]);
		}
		else {
			input.leftLR = (input.gamepadButtons[SDL_CONTROLLER_BUTTON_DPAD_RIGHT] - input.gamepadButtons[SDL_CONTROLLER_BUTTON_DPAD_LEFT]);
			input.leftUD = (input.gamepadButtons[SDL_CONTROLLER_BUTTON_DPAD_DOWN] - input.gamepadButtons[SDL_CONTROLLER_BUTTON_DPAD_UP]);
		}

		//for compatability with the analog sticks, direction along an axis will be either 0, gamepad_AXIS_MAX, or gamepad_AXIS_MIN
		if (input.leftLR > 0) {
			input.leftLR = GAMEPAD_AXIS_MAX;
		}
		if (input.leftLR < 0) {
			input.leftLR = GAMEPAD_AXIS_MIN;
		}
		if (input.leftUD > 0) {
			input.leftUD = GAMEPAD_AXIS_MAX;
		}
		if (input.leftUD < 0) {
			input.leftUD = GAMEPAD_AXIS_MIN;
		}
	}

	//look-around input
	//mouse is also used for this in this game, but that's kept seperate in this case b/c consolidating those two things seems unwise
	if (input.gamepad != NULL && (abs(input.gamepadAxes[SDL_CONTROLLER_AXIS_RIGHTX]) > input.deadzone || abs(input.gamepadAxes[SDL_CONTROLLER_AXIS_RIGHTY]) > input.deadzone)) {
		//analog stick input
		input.rightLR = input.gamepadAxes[SDL_CONTROLLER_AXIS_RIGHTX];
		input.rightUD = input.gamepadAxes[SDL_CONTROLLER_AXIS_RIGHTY];
	}

	//normalize directions
	//technically off by ~.00001 when in a negative direction but who cares
	if (abs(input.leftLR) > input.deadzone || abs(input.leftUD > input.deadzone)) {
		input.leftLR /= GAMEPAD_AXIS_MAX;
		input.leftUD /= GAMEPAD_AXIS_MAX;
	}

	if (abs(input.rightLR) > input.deadzone || abs(input.rightUD > input.deadzone)) {
		input.rightLR /= GAMEPAD_AXIS_MAX;
		input.rightUD /= GAMEPAD_AXIS_MAX;
	}

	//buttons

	//left click or r1 to fire/confirm
	if (input.mouse.buttons[SDL_BUTTON_LEFT] || input.gamepadButtons[SDL_CONTROLLER_BUTTON_RIGHTSHOULDER]) {
		input.fire = true;
	}

	//right click or circle to go back
	if (input.mouse.buttons[SDL_BUTTON_RIGHT] || input.gamepadButtons[SDL_CONTROLLER_BUTTON_B]) {
		input.dash = true;
	}

	//esc or options to pause
	if (input.keyboard[SDL_SCANCODE_ESCAPE] || input.gamepadButtons[SDL_CONTROLLER_BUTTON_START]) {
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
			initGamepad();
			break;

		case SDL_CONTROLLERBUTTONUP:
			doGamepadButtonUp(&event.cbutton);
			break;

		case SDL_CONTROLLERBUTTONDOWN:
			doGamepadButtonDown(&event.cbutton);
			break;

		case SDL_CONTROLLERAXISMOTION:
			doGamepadAxis(&event.caxis);
			break;

		case SDL_MOUSEBUTTONUP:
			doMouseButtonUp(&event.button);
			break;

		case SDL_MOUSEBUTTONDOWN:
			doMouseButtonDown(&event.button);
			break;

		case SDL_MOUSEWHEEL:

			break;

		default:
			break;
		}
	}

	//handle scroll wheel
	if (event.type == SDL_MOUSEWHEEL) {
		input.mouse.wheel = event.wheel.y;

		//determine most recently used controller (keyboard and mouse or gamepad)
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
