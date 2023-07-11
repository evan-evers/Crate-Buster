#include "common.h"

#include "bullets.h"
#include "player.h"
#include "stage.h"

extern App app;
extern InputManager input;
extern Player* player;
extern Stage stage;

static void logic(void);
static void draw(void);

void initStage(void) {
	//set function pointers
	app.delegate.logic = logic;
	app.delegate.draw = draw;

	//initialize lists
	stage.bulletHead = NULL;
	stage.bulletTail = stage.bulletHead;

	initPlayer(SCREEN_WIDTH * 0.5, SCREEN_HEIGHT * 0.5);

	initBullets();
}

static void logic(void) {
	updatePlayer();

	updateBullets();
}

static void draw(void) {
	drawBullets();

	drawPlayer();
}

void deleteStage(void) {
	deleteBullets();

	deletePlayer();
}