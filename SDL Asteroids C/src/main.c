/*
* The project main.
*/

#include "common.h"

#include "background.h"
#include "cursor.h"
#include "draw.h"
#include "init.h"
#include "input.h"
#include "mainMenu.h"
#include "player.h"
#include "stage.h"
#include "widgets.h"

App app;
Background background;
InputManager input;
Player* player;
Stage stage;

int main(int argc, char* argv[]) {
	//clear/initialize important structs
	memset(&app, 0, sizeof(App));
	app.quit = false;	//not technically necessary but I like this being explicit
	app.debug = false;
	memset(&input, 0, sizeof(InputManager));

	//initialize SDL
	if (!initSDL()) {
		printf("ERROR: SDL failed to initialize.\n");
		close();
		return 1;	//return with error code
	}

	//initialize game
	if (!initGame()) {
		printf("ERROR: Game failed to initialize.\n");
		close();
		return 1;	//return with error code
	}

	//prevent something instantly happening because the user pressed something while the game was first loading
	resetInput();

	//since main menu and gameplay use the same background, init it here
	initBackground();

	//start game on main menu
	initMainMenu();

	while (!app.quit) {
		handleInput();

		app.delegate.logic();

		prepareScene();

		app.delegate.draw();

		drawCursor();	//cursor will be drawn over everything else in the scene

		presentScene();
	}

	//clean up
	deleteBackground();

	close();

	return 0;
}