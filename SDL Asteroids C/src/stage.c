#include "common.h"

#include "player.h"
#include "stage.h"

extern App app;
extern InputManager input;
extern Player* player;
extern Stage stage;

void initStage(void);
static void logic(void);
static void draw(void);
void deleteStage(void);

void initStage(void) {
	//initialize/reset stage
	memset(&stage, 0, sizeof(Stage));

	//set function pointers
	app.delegate.logic = logic;
	app.delegate.draw = draw;

	//initialize stage lists
	stage.bulletHead = stage.bulletTail = NULL;
	stage.crateHead = stage.crateTail = NULL;

	initPlayer(SCREEN_WIDTH * 0.5, SCREEN_HEIGHT * 0.5);

	initCrates(8);
}

static void logic(void) {
	updatePlayer();

	updateCrates();

	updateBullets();
}

static void draw(void) {
	drawBullets();

	drawCrates();

	drawPlayer();
}

void deleteStage(void) {
	deleteCrates();

	deleteBullets();

	deletePlayer();
}