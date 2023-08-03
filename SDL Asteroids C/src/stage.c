#include "common.h"

#include "background.h"
#include "player.h"
#include "powerups.h"
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
	stage.enemyHead = stage.enemyTail = NULL;
	stage.particleHead = stage.particleTail = NULL;

	//initialize objects that must exist at start of level
	initBackground();

	initPlayer(SCREEN_WIDTH * 0.5, SCREEN_HEIGHT * 0.5);

	initCrates(8);

	initPowerups();
}

static void logic(void) {
	updatePlayer();

	updateParticles();

	updateCrates();

	updateBullets();

	updateEnemies();

	spawnEnemies();

	//increment timer
	++stage.timer;
}

static void draw(void) {
	drawBackground();

	drawParticles();

	drawCrates();

	drawEnemies();

	drawBullets();

	drawPlayer();
}

void deleteStage(void) {
	deleteBackground();

	deleteParticles();

	deleteCrates();

	deleteBullets();

	deletePlayer();

	deleteEnemies();

	deletePowerups();
}