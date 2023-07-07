/*
* The project main.
*/

#include "common.h"

#include "draw.h"
#include "init.h"
#include "input.h"
#include "player.h"

App app;
InputManager input;

int main(int argc, char* argv[]) {
	//clear/initialize important structs
	memset(&app, 0, sizeof(App));
	app.quit = false;	//not technically necessary but I like this being explicit
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

	Player* player = initPlayer(100,100);

	while (!app.quit) {
		handleInput();

		//logic here
		updatePlayer(player);	//here temporarily

		prepareScene();

		//draw here
		drawPlayer(player);	//here temporarily

		presentScene();
	}

	//clean up
	deletePlayer(player);	//this should go in a deleteStage or deleteRoom function once that's up and running
	close();

	return 0;
}