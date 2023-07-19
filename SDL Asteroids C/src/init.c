#include "common.h"

#include "SDL_image.h"
#include "SDL_mixer.h"

#include "init.h"
#include "input.h"

extern App app;
extern InputManager input;

bool initSDL(void);
bool initGame(void);
void close(void);

//Initializes SDL and its subsystems. Returns true on successful init and false on unsuccessful init.
bool initSDL(void) {
	bool success = true;

	//Initialize SDL
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
		printf("ERROR - Couldn't initialize SDL: %s\n", SDL_GetError());
		success = false;
	}

	//Tell SDL to use linear filtering when scaling textures if possible
	if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest")) {
		printf("WARNING - nearest pixel sampling could not be enabled.\n");
	}

	//Create program window wherever the OS wants it (args 2 and 3)
	//NOTE: Window title declared here
	app.window = SDL_CreateWindow("Working Title", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
	if (app.window == NULL) {
		printf("ERROR - Window could not be created: %s\n", SDL_GetError());
		success = false;
	}

	//Create renderer. Let SDL use whatever graphics acceleration device it wants (arg 2).
	//Using vsync to cap framerate
	app.renderer = SDL_CreateRenderer(app.window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (app.renderer == NULL) {
		printf("ERROR - Renderer could not be created: %s\n", SDL_GetError());
		success = false;
	}
	else {
		//make window scale with screen size changes
		SDL_RenderSetLogicalSize(app.renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
		//make sure mouse coordinates correspond to relative screen size
		if (!SDL_SetHint(SDL_HINT_MOUSE_RELATIVE_SCALING, "1")) {
			printf("WARNING - relative mouse scaling could not be enabled.\n");
		}

		//start fullscreen
		//SDL_SetWindowFullscreen(app.window, SDL_WINDOW_FULLSCREEN_DESKTOP);
	}

	//initialize SDL image
	if (IMG_Init(IMG_INIT_PNG) < 0) {
		printf("ERROR - Couldn't initialize SDL_image: %s\n", SDL_GetError());
		success = false;
	}

	//Initialize SDL audio
	if (Mix_OpenAudio(48000, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
		printf("ERROR - Couldn't initialize SDL_mixer: %s\n", SDL_GetError());
		success = false;
	}

	//Allocate sound channels
	//This one doesn't seem to have an error condition, according to SDL documentation
	Mix_AllocateChannels(MAX_SND_CHANNELS);

	return success;
}

//Initializes game variables and systems.
bool initGame(void) {
	bool success = true;

	//set window size ratios for proper mouse interaction
	//code adapted from handleWindowResize from input.c
	int w, h;
	SDL_GetWindowSize(app.window, &w, &h);

	if (w != 0 && h != 0) {
		//find letterboxing
		app.windowPaddingW = 0;
		app.windowPaddingH = 0;

		//horizontal letterboxing
		if (((float)w / (float)h) > ASPECT_RATIO)
			app.windowPaddingW = w - ASPECT_RATIO * (double)h;

		//vertical letterboxing
		if (((float)h / (float)w) > ASPECT_RATIO_INV)
			app.windowPaddingH = h - ASPECT_RATIO_INV * (double)w;

		//set ratios only according to the non-letterbox parts of the screen	
		app.windowPixelRatioW = (double)SCREEN_WIDTH / (double)(w - app.windowPaddingW);
		app.windowPixelRatioH = (double)SCREEN_HEIGHT / (double)(h - app.windowPaddingH);
	}

	//initialize input variables
	input.gamepad = NULL;
	input.deadzone = 8000;

	//initialize joypad
	initGamepad();

	//Load the sprite atlases to be used
	app.fontsAndUI = initSpriteAtlas("gfx/AsteroidsCloneFontsAndUI.png");
	app.gameplaySprites = initSpriteAtlas("gfx/AsteroidsCloneSpriteSheet.png");

	if (app.fontsAndUI == NULL) {
		printf("ERROR - Fonts and UI could not be loaded: %s\n", IMG_GetError());
		success = false;
	}
	if (app.gameplaySprites == NULL) {
		printf("ERROR - Gameplay sprites could not be loaded: %s\n", IMG_GetError());
		success = false;
	}

	//randomize
	srand(time(NULL));

	return success;
}

//Clean up textures and other dynamically allocated things
//Then clean up renderer and window
//Then quit subsystems
void close(void) {
	//Free resources here (pointers are NULLed within the functions)
	deleteSpriteAtlas(app.fontsAndUI);
	deleteSpriteAtlas(app.gameplaySprites);

	//close joypad
	SDL_GameControllerClose(input.gamepad);
	input.gamepad = NULL;

	//Clean up renderer
	SDL_DestroyRenderer(app.renderer);
	app.renderer = NULL;

	//Clean up window
	SDL_DestroyWindow(app.window);
	app.window = NULL;

	//Quit subsystems
	Mix_Quit();
	IMG_Quit();
	SDL_Quit();
}