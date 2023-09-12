#include "common.h"

#include "input.h"

extern App app;
extern InputManager input;

void resetInput(void);
static void handleWindowResize(const SDL_WindowEvent* event);
static void doKeyUp(const SDL_KeyboardEvent* event);
static void doKeyDown(const SDL_KeyboardEvent* event);
void initGamepad(void);
static void doGamepadButtonUp(const SDL_ControllerButtonEvent* event);
static void doGamepadButtonDown(const SDL_ControllerButtonEvent* event);
static void doGamepadAxis(const SDL_ControllerAxisEvent* event);
static void doMouseButtonUp(const SDL_MouseButtonEvent* event);
static void doMouseButtonDown(const SDL_MouseButtonEvent* event);
static void doGameplayInput(void);
void handleInput(void);

static const int INPUT_BUFFER_MAX = 5;	//in frames/updates
static const int DEADZONE = 8000;	//deadzone for general applications
static const int STICK_PRESSED_DEADZONE = 25000;	//a special deadzone for testing if a movement of the joystick should count as a directional press
static int prevAxisValues[SDL_CONTROLLER_AXIS_MAX] = { 0 };	//holds the values of the gamepad axes from the last frame, to allow directional pressed variables to be activated by moving the joysticks
static int prevMouseX;	//values to check if the mouse was moved this tick
static int prevMouseY;

//resets input
//should be called at game start and whenever input needs to be reset
void resetInput(void) {
	//go through and set all input to nothing
	for (int i = 0; i < MAX_KEYBOARD_KEYS; ++i) {
		input.keyboard[i] = IS_NONE;
	}
	for (int i = 0; i < SDL_CONTROLLER_BUTTON_MAX; ++i) {
		input.gamepadButtons[i] = IS_NONE;
	}
	for (int i = 0; i < MAX_MOUSE_BUTTONS; ++i) {
		input.mouse.buttons[i] = IS_NONE;
	}
}

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
	
	//input can get "stuck" when resizing window if an input was held down during resize, so reset input
	resetInput();
}

//keyboard control handling
static void doKeyUp(const SDL_KeyboardEvent* event) {
	if (event->repeat == 0 && event->keysym.scancode >= 0 && event->keysym.scancode < MAX_KEYBOARD_KEYS) {
		//set none and released flags to 1
		//released flag will be set back to 0 on next update in handleInput
		input.keyboard[event->keysym.scancode] = IS_RELEASED;
	}
}

static void doKeyDown(const SDL_KeyboardEvent* event) {
	if (event->repeat == 0 && event->keysym.scancode >= 0 && event->keysym.scancode < MAX_KEYBOARD_KEYS) {
		//set held and pressed flags to 1
		//pressed flag will be set back to 0 on next update in handleInput
		input.keyboard[event->keysym.scancode] = IS_HELD | IS_PRESSED;

		//update most recently used controller (keyboard and mouse or gamepad)
		input.lastControllerType = LCT_KEYBOARD_AND_MOUSE;

		//update last pressed key for input remapping
		input.lastKeyPressed = event->keysym.scancode;
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
static void doGamepadButtonUp(const SDL_ControllerButtonEvent* event) {
	if (event->state == SDL_RELEASED && event->button < SDL_CONTROLLER_BUTTON_MAX) {
		//set none and released flags to 1
		//released flag will be set back to 0 on next update in handleInput
		input.gamepadButtons[event->button] = IS_RELEASED;
	}
}

static void doGamepadButtonDown(const SDL_ControllerButtonEvent* event) {
	if (event->state == SDL_PRESSED && event->button < SDL_CONTROLLER_BUTTON_MAX) {
		//set held and pressed flags to 1
		//pressed flag will be set back to 0 on next update in handleInput
		input.gamepadButtons[event->button] = IS_HELD | IS_PRESSED;

		//update most recently used controller (keyboard and mouse or gamepad)
		input.lastControllerType = LCT_GAMEPAD;

		//update last pressed key for input remapping
		input.lastGamepadButtonPressed = event->button;
	}
}

static void doGamepadAxis(const SDL_ControllerAxisEvent* event) {
	if (event->axis < SDL_CONTROLLER_AXIS_MAX) {
		input.gamepadAxes[event->axis] = event->value;

		//NOTE: don't update input.lastControllerType here
		//because this gets called every frame (presumably b/c of stick drift)
	}
}

//mouse control handling (some is also done in handleInput)
static void doMouseButtonUp(const SDL_MouseButtonEvent* event) {
	//set none and released flags to 1
	//released flag will be set back to 0 on next update in handleInput
	input.mouse.buttons[event->button] = IS_RELEASED;
}

static void doMouseButtonDown(const SDL_MouseButtonEvent* event) {
	//set held and pressed flags to 1
	//pressed flag will be set back to 0 on next update in handleInput
	input.mouse.buttons[event->button] = IS_HELD | IS_PRESSED;

	//update most recently used controller (keyboard and mouse or gamepad)
	input.lastControllerType = LCT_KEYBOARD_AND_MOUSE;

	//update last pressed key for input remapping
	input.lastMouseButtonPressed = event->button;
}

static void doMouseWheel(const SDL_MouseWheelEvent *event) {
	//store vertical scroll data in input.mouse.wheel
	//multiple increments of up or down on the same tick will be ignored
	if (event->y > 0)
		input.mouse.wheel = 1;
	if (event->y < 0)
		input.mouse.wheel = -1;

	//update most recently used controller (keyboard and mouse or gamepad)
	input.lastControllerType = LCT_KEYBOARD_AND_MOUSE;
}

//handles gameplay input
//filters joystick input through deadzones
static void doGameplayInput(void) {
	//initialize some in unpressed/neutral positions
	//button controls aren't initialized like this because their decrementing buffers naturally reset them
	input.leftLR = 0;
	input.leftUD = 0;
	input.rightLR = 0;
	input.rightUD = 0;

	//directional input
	if (input.gamepad != NULL && (abs(input.gamepadAxes[SDL_CONTROLLER_AXIS_LEFTX]) >= DEADZONE || abs(input.gamepadAxes[SDL_CONTROLLER_AXIS_LEFTY]) >= DEADZONE)) {
		//analog stick input
		input.leftLR = input.gamepadAxes[SDL_CONTROLLER_AXIS_LEFTX];
		input.leftUD = input.gamepadAxes[SDL_CONTROLLER_AXIS_LEFTY];

		//allow both joysticks to activate directional presses
		//if a joystick axis was not past the deadzone in one direction on the last frame, but is past the deadzone on this frame, 
		//but is past the axis on this frame, that's counted as a press
		//just like in the buttons section for directional pressed variables, only left or right/up or down can be pressed; never both
		//down and right are prioritized over up and left for no reason
		if (prevAxisValues[SDL_CONTROLLER_AXIS_LEFTX] < STICK_PRESSED_DEADZONE && input.gamepadAxes[SDL_CONTROLLER_AXIS_LEFTX] >= STICK_PRESSED_DEADZONE)
			input.rightPressed = INPUT_BUFFER_MAX;
		else if (prevAxisValues[SDL_CONTROLLER_AXIS_LEFTX] > -STICK_PRESSED_DEADZONE && input.gamepadAxes[SDL_CONTROLLER_AXIS_LEFTX] <= -STICK_PRESSED_DEADZONE)
			input.leftPressed = INPUT_BUFFER_MAX;
		if (prevAxisValues[SDL_CONTROLLER_AXIS_LEFTY] < STICK_PRESSED_DEADZONE && input.gamepadAxes[SDL_CONTROLLER_AXIS_LEFTY] > STICK_PRESSED_DEADZONE)
			input.downPressed = INPUT_BUFFER_MAX;
		else if (prevAxisValues[SDL_CONTROLLER_AXIS_LEFTY] > -STICK_PRESSED_DEADZONE && input.gamepadAxes[SDL_CONTROLLER_AXIS_LEFTY] <= -STICK_PRESSED_DEADZONE)
			input.upPressed = INPUT_BUFFER_MAX;

		//this goes here instead of in an input function, because the deadzones are checked here
		input.lastControllerType = LCT_GAMEPAD;
	} else {
		//do keyboard/d-pad input if there's no controller plugged in or if there's no analog stick input

		//find direction
		if (input.lastControllerType == LCT_KEYBOARD_AND_MOUSE) {
			//check for held flags; balance input automatically
			input.leftLR = ((input.keyboard[SDL_SCANCODE_D] & IS_HELD) - (input.keyboard[SDL_SCANCODE_A] & IS_HELD));
			input.leftUD = ((input.keyboard[SDL_SCANCODE_S] & IS_HELD) - (input.keyboard[SDL_SCANCODE_W] & IS_HELD));
		} else {
			//check for held flags; balance input automatically
			input.leftLR = ((input.gamepadButtons[SDL_CONTROLLER_BUTTON_DPAD_RIGHT] & IS_HELD) - (input.gamepadButtons[SDL_CONTROLLER_BUTTON_DPAD_LEFT] & IS_HELD));
			input.leftUD = ((input.gamepadButtons[SDL_CONTROLLER_BUTTON_DPAD_DOWN] & IS_HELD) - (input.gamepadButtons[SDL_CONTROLLER_BUTTON_DPAD_UP] & IS_HELD));
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

		//set last pressed direction buttons
		if (input.lastControllerType == LCT_KEYBOARD_AND_MOUSE) {
			//each directional axis uses an if-else block
			//to prevent the game from getting leftPressed and rightPressed or upPressed and downPressed on the same frame
			//this only makes a difference in very few kinds of games, like fighitng games meant for high-level competitive play, but whatever
			//down and right are prioritized over up and left for no reason whatsoever
			if (input.keyboard[SDL_SCANCODE_D] & IS_PRESSED)
				input.rightPressed = INPUT_BUFFER_MAX;
			else if (input.keyboard[SDL_SCANCODE_A] & IS_PRESSED)
				input.leftPressed = INPUT_BUFFER_MAX;
			if (input.keyboard[SDL_SCANCODE_S] & IS_PRESSED || input.mouse.wheel == -1)
				input.downPressed = INPUT_BUFFER_MAX;
			else if (input.keyboard[SDL_SCANCODE_W] & IS_PRESSED || input.mouse.wheel == 1)
				input.upPressed = INPUT_BUFFER_MAX;
		} else {
			if (input.gamepadButtons[SDL_CONTROLLER_BUTTON_DPAD_RIGHT] & IS_PRESSED)
				input.rightPressed = INPUT_BUFFER_MAX;
			else if (input.gamepadButtons[SDL_CONTROLLER_BUTTON_DPAD_LEFT] & IS_PRESSED)
				input.leftPressed = INPUT_BUFFER_MAX;
			if (input.gamepadButtons[SDL_CONTROLLER_BUTTON_DPAD_DOWN] & IS_PRESSED)
				input.downPressed = INPUT_BUFFER_MAX;
			else if (input.gamepadButtons[SDL_CONTROLLER_BUTTON_DPAD_UP] & IS_PRESSED)
				input.upPressed = INPUT_BUFFER_MAX;
		}
	}

	//look-around input
	//mouse is also used for this in this game, but that's kept seperate in this case b/c consolidating those two things seems unwise
	if (input.gamepad != NULL && (abs(input.gamepadAxes[SDL_CONTROLLER_AXIS_RIGHTX]) >= DEADZONE || abs(input.gamepadAxes[SDL_CONTROLLER_AXIS_RIGHTY]) >= DEADZONE)) {
		//analog stick input
		input.rightLR = input.gamepadAxes[SDL_CONTROLLER_AXIS_RIGHTX];
		input.rightUD = input.gamepadAxes[SDL_CONTROLLER_AXIS_RIGHTY];

		//this goes here instead of in an input function, because the deadzones are checked here
		input.lastControllerType = LCT_GAMEPAD;
	}

	//normalize directions
	//technically off by ~.00001 when in a negative direction but who cares
	if (abs(input.leftLR) > DEADZONE || abs(input.leftUD) > DEADZONE) {
		input.leftLR /= GAMEPAD_AXIS_MAX;
		input.leftUD /= GAMEPAD_AXIS_MAX;
	}

	if (abs(input.rightLR) > DEADZONE || abs(input.rightUD) > DEADZONE) {
		input.rightLR /= GAMEPAD_AXIS_MAX;
		input.rightUD /= GAMEPAD_AXIS_MAX;
	}

	//buttons

	//decrement buffers
	--input.leftPressed;
	--input.rightPressed;
	--input.upPressed;
	--input.downPressed;
	--input.backspacePressed;
	--input.fire;
	--input.firePressed;
	--input.dash;
	--input.dashPressed;
	--input.pause;
	--input.pausePressed;

	//for text input
	if (input.keyboard[SDL_SCANCODE_BACKSPACE] & IS_PRESSED)
		input.backspacePressed = INPUT_BUFFER_MAX;

	//left click, enter, bottom face or right shoulder to fire/confirm
	if ((input.mouse.buttons[SDL_BUTTON_LEFT] & IS_HELD) || (input.keyboard[SDL_SCANCODE_RETURN] & IS_HELD) || (input.gamepadButtons[SDL_CONTROLLER_BUTTON_A] & IS_HELD) || (input.gamepadButtons[SDL_CONTROLLER_BUTTON_RIGHTSHOULDER] & IS_HELD))
		input.fire = INPUT_BUFFER_MAX;
	if ((input.mouse.buttons[SDL_BUTTON_LEFT] & IS_PRESSED) || (input.keyboard[SDL_SCANCODE_RETURN] & IS_PRESSED) || (input.gamepadButtons[SDL_CONTROLLER_BUTTON_A] & IS_PRESSED) || (input.gamepadButtons[SDL_CONTROLLER_BUTTON_RIGHTSHOULDER] & IS_PRESSED))
		input.firePressed = INPUT_BUFFER_MAX;

	//right click or circle to go back
	if ((input.mouse.buttons[SDL_BUTTON_RIGHT] & IS_HELD) || (input.gamepadButtons[SDL_CONTROLLER_BUTTON_B] & IS_HELD))
		input.dash = INPUT_BUFFER_MAX;
	if ((input.mouse.buttons[SDL_BUTTON_RIGHT] & IS_PRESSED) || (input.gamepadButtons[SDL_CONTROLLER_BUTTON_B] & IS_PRESSED))
		input.dashPressed = INPUT_BUFFER_MAX;

	//esc or options to pause
	if ((input.keyboard[SDL_SCANCODE_ESCAPE] & IS_HELD) || (input.gamepadButtons[SDL_CONTROLLER_BUTTON_START] & IS_HELD))
		input.pause = INPUT_BUFFER_MAX;
	if ((input.keyboard[SDL_SCANCODE_ESCAPE] & IS_PRESSED) || (input.gamepadButtons[SDL_CONTROLLER_BUTTON_START] & IS_PRESSED))
		input.pausePressed = INPUT_BUFFER_MAX;



	//store values of gamepad axes on this frame to allow joystick presses to activate directional pressed variables
	prevAxisValues[SDL_CONTROLLER_AXIS_LEFTX] = input.gamepadAxes[SDL_CONTROLLER_AXIS_LEFTX];
	prevAxisValues[SDL_CONTROLLER_AXIS_LEFTY] = input.gamepadAxes[SDL_CONTROLLER_AXIS_LEFTY];
	prevAxisValues[SDL_CONTROLLER_AXIS_RIGHTX] = input.gamepadAxes[SDL_CONTROLLER_AXIS_RIGHTX];
	prevAxisValues[SDL_CONTROLLER_AXIS_RIGHTY] = input.gamepadAxes[SDL_CONTROLLER_AXIS_RIGHTY];
	//trigger axes would go here if I was using them
}

void handleInput(void) {
	SDL_Event event;

	//go through and set IS_PRESSED and IS_RELEASED flags to 0
	for (int i = 0; i < MAX_KEYBOARD_KEYS; ++i) {
		if (input.keyboard[i] & IS_PRESSED)
			input.keyboard[i] &= ~IS_PRESSED;
		if (input.keyboard[i] & IS_RELEASED)
			input.keyboard[i] &= ~IS_RELEASED;
	}
	for (int i = 0; i < SDL_CONTROLLER_BUTTON_MAX; ++i) {
		if (input.gamepadButtons[i] & IS_PRESSED)
			input.gamepadButtons[i] &= ~IS_PRESSED;
		if (input.gamepadButtons[i] & IS_RELEASED)
			input.gamepadButtons[i] &= ~IS_RELEASED;
	}
	for (int i = 0; i < MAX_MOUSE_BUTTONS; ++i) {
		if (input.mouse.buttons[i] & IS_PRESSED)
			input.mouse.buttons[i] &= ~IS_PRESSED;
		if (input.mouse.buttons[i] & IS_RELEASED)
			input.mouse.buttons[i] &= ~IS_RELEASED;
	}
	input.mouse.wheel = 0;	//reset scroll wheel

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

		case SDL_TEXTINPUT:
			//copy any text input into input.inputText
			STRNCPY(input.inputText, event.text.text, MAX_INPUT_LENGTH);
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
			doMouseWheel(&event.wheel);
			break;

		default:
			break;
		}
	}

	//handle mouse

	//get previous mouse position
	prevMouseX = input.mouse.x;
	prevMouseY = input.mouse.y;

	//get mouse position
	SDL_GetMouseState(&input.mouse.x, &input.mouse.y);

	//adjust mouse position according to how the screen's been stretched (letterboxing taken into account by the windowPadding vars)
	input.mouse.x = ((float)input.mouse.x - (float)app.windowPaddingW * 0.5) * app.windowPixelRatioW;
	input.mouse.y = ((float)input.mouse.y - (float)app.windowPaddingH * 0.5) * app.windowPixelRatioH;

	//check for a movement of the mouse and record it if it happened
	if (input.mouse.x != prevMouseX || input.mouse.y != prevMouseY) {
		input.mouseWasMoved = true;
		input.lastControllerType = LCT_KEYBOARD_AND_MOUSE;
	}
	else
		input.mouseWasMoved = false;

	//package raw input data into gameplay input interface
	doGameplayInput();
}
