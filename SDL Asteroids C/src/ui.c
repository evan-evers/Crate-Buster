#include "common.h"

#include "background.h"
#include "draw.h"
#include "fonts.h"
#include "highscores.h"
#include "player.h"
#include "stage.h"
#include "ui.h"
#include "widgets.h"

extern App app;
extern Player *player;
extern Stage stage;
extern PLAYER_HP_MAX;

//vars for drawing beginning and end UI
static const int startTextVertMargin = 45;

//text buffer for drawStageEndUI
char scoreTextEnd[40];

//margins for drawing gameplay UI
static const int horzMargin = 8;
static const int bottomVertMargin = 20;
static const int topVertMargin = 8;
static const int heartHorzMargin = 28;

//text buffers for drawGameplayUI
char scoreText[40];
char stageText[40];

//sprites for gameplay UI
static SpriteStatic *heartFull;
static SpriteStatic *heartEmpty;

//sprites for main menu UI
static SpriteStatic *title;

//sprites for how to play UI
static SpriteStatic *leftStickGraphic;
static SpriteStatic *dpadGraphic;
static SpriteStatic *keyBackgroundGraphic;	//background square to put text on to indicate a keyboard key
static SpriteStatic *rightStickGraphic;
static SpriteStatic *mouseGraphic;
static SpriteStatic *leftMouseButtonGraphic;
static SpriteStatic *rightShoulderButtonGraphic;
static SpriteStatic *bottomFaceButtonGraphic;

static SpriteStatic *crateGraphic;
static SpriteStatic *enemyShipGraphic;
static SpriteStatic *enemyBulletGraphic;
static SpriteStatic *playerShipGraphic;
static SpriteStatic *playerBulletGraphic;
static SpriteStatic *scrapGraphic1;
static SpriteStatic *scrapGraphic2;
static SpriteStatic *scrapGraphic3;
static SpriteStatic *scrapGraphic4;
static SpriteStatic *powerupBaseGraphic;
static SpriteStatic *powerupLetterNGraphic;
static SpriteStatic *powerupLetterEGraphic;
static SpriteStatic *powerupLetterBGraphic;
static SpriteStatic *powerupLetterSGraphic;

//positional variables for how to play UI
static const int LEFT_X = SCREEN_WIDTH * 0.25;
static const int RIGHT_X = SCREEN_WIDTH * 0.75;

void initGameplayUI(void) {
	heartFull = initSpriteStatic(app.fontsAndUI, 12, 19, 2, 2, SC_TOP_LEFT);
	heartEmpty = initSpriteStatic(app.fontsAndUI, 14, 19, 2, 2, SC_TOP_LEFT);
}

void drawStageStartUI(void) {
	//draw ready text in center of screen
	drawTextDropShadow("GET READY...", SCREEN_WIDTH * 0.5, SCREEN_HEIGHT * 0.5 - startTextVertMargin, PALETTE_WHITE, TAH_CENTER, NULL, PALETTE_BLACK, 1);
}

void drawGameplayUI(void) {
	//draw player health in top left
	for (int i = 0; i < PLAYER_HP_MAX; ++i) {
		if(i < player->hp)
			blitSpriteStatic(heartFull, horzMargin + i * heartHorzMargin, topVertMargin);
		else
			blitSpriteStatic(heartEmpty, horzMargin + i * heartHorzMargin, topVertMargin);
	}

	//draw player score in top right
	snprintf(scoreText, 40, "Score: %d", stage.score);
	drawTextDropShadow(scoreText, SCREEN_WIDTH - horzMargin, topVertMargin, PALETTE_WHITE, TAH_RIGHT, NULL, PALETTE_BLACK, 1);

	//draw current weapon's name in bottom left
	switch (player->weaponType) {
	case(BT_NORMAL):
		drawTextDropShadow( "WEAPON: NORMAL", horzMargin, SCREEN_HEIGHT - bottomVertMargin, PALETTE_WHITE, TAH_LEFT, NULL, PALETTE_BLACK, 1);
		break;
	case(BT_ERRATIC):
		drawTextDropShadow("WEAPON: ERRATIC", horzMargin, SCREEN_HEIGHT - bottomVertMargin, PALETTE_WHITE, TAH_LEFT, NULL, PALETTE_BLACK, 1);
		break;
	case(BT_BOUNCER):
		drawTextDropShadow("WEAPON: BOUNCER", horzMargin, SCREEN_HEIGHT - bottomVertMargin, PALETTE_WHITE, TAH_LEFT, NULL, PALETTE_BLACK, 1);
		break;
	case(BT_SHOTGUN):
		drawTextDropShadow("WEAPON: SHOTGUN", horzMargin, SCREEN_HEIGHT - bottomVertMargin, PALETTE_WHITE, TAH_LEFT, NULL, PALETTE_BLACK, 1);
		break;
	}

	//draw stage number in bottom right
	snprintf(stageText, 40, "STAGE: %d", stage.level);
	drawTextDropShadow(stageText, SCREEN_WIDTH - horzMargin, SCREEN_HEIGHT - bottomVertMargin, PALETTE_WHITE, TAH_RIGHT, NULL, PALETTE_BLACK, 1);
}

void drawStageEndUI(void) {
	drawTextDropShadow("WELL DONE!", SCREEN_WIDTH * 0.5, SCREEN_HEIGHT * 0.5 - startTextVertMargin, PALETTE_WHITE, TAH_CENTER, NULL, PALETTE_BLACK, 1);
	sprintf(scoreTextEnd, "YOUR SCORE: %d", stage.score);
	drawTextDropShadow(scoreTextEnd, SCREEN_WIDTH * 0.5, SCREEN_HEIGHT * 0.5, PALETTE_WHITE, TAH_CENTER, NULL, PALETTE_BLACK, 1);
	drawTextDropShadow("PRESS FIRE TO PLAY NEXT STAGE", SCREEN_WIDTH * 0.5, SCREEN_HEIGHT * 0.5 + startTextVertMargin, PALETTE_WHITE, TAH_CENTER, NULL, PALETTE_BLACK, 1);
}

void drawStageGameOverUI(void) {
	drawTextDropShadow("GAME OVER", SCREEN_WIDTH * 0.5, SCREEN_HEIGHT * 0.5 - 15, PALETTE_WHITE, TAH_CENTER, NULL, PALETTE_BLACK, 1);
	sprintf(scoreTextEnd, "YOUR SCORE: %d", stage.score);
	drawTextDropShadow(scoreTextEnd, SCREEN_WIDTH * 0.5, SCREEN_HEIGHT * 0.5 + 15, PALETTE_WHITE, TAH_CENTER, NULL, PALETTE_BLACK, 1);

	drawWidgets(NULL);
}

void drawPausedUI(void) {
	//draw a semitransparent black rectangle over the screen
	//stuff like this is why I need a rectangle drawing function
	SDL_Rect screenDarken = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
	SDL_SetRenderDrawBlendMode(app.renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(app.renderer, PALETTE_BLACK.r, PALETTE_BLACK.g, PALETTE_BLACK.b, 191);
	SDL_RenderFillRect(app.renderer, &screenDarken);

	drawWidgets(NULL);
}

void deleteGameplayUI(void) {
	free(heartFull);
	free(heartEmpty);
}

//initializes stuff for main menu and its submenus/options
void initMainMenuUI(void) {
	title = initSpriteStatic(app.fontsAndUI, 0, 23, 17, 8, SC_TOP_CENTER);

	leftStickGraphic = initSpriteStatic(app.fontsAndUI, 12, 18, 1, 1, SC_CENTER);
	dpadGraphic = initSpriteStatic(app.fontsAndUI, 7, 21, 1, 1, SC_CENTER);
	keyBackgroundGraphic = initSpriteStatic(app.fontsAndUI, 15, 18, 1, 1, SC_CENTER);
	rightStickGraphic = initSpriteStatic(app.fontsAndUI, 13, 18, 1, 1, SC_CENTER);
	mouseGraphic = initSpriteStatic(app.fontsAndUI, 9, 22, 1, 1, SC_CENTER);
	leftMouseButtonGraphic = initSpriteStatic(app.fontsAndUI, 7, 22, 1, 1, SC_CENTER);
	rightShoulderButtonGraphic = initSpriteStatic(app.fontsAndUI, 11, 20, 1, 1, SC_CENTER);
	bottomFaceButtonGraphic = initSpriteStatic(app.fontsAndUI, 8, 19, 1, 1, SC_CENTER);

	crateGraphic = initSpriteStatic(app.gameplaySprites, 0, 4, 3, 3, SC_CENTER);
	enemyShipGraphic = initSpriteStatic(app.gameplaySprites, 6, 7, 4, 4, SC_CENTER);
	enemyBulletGraphic = initSpriteStatic(app.gameplaySprites, 16, 12, 2, 1, SC_CENTER);
	playerShipGraphic = initSpriteStatic(app.gameplaySprites, 0, 11, 3, 3, SC_CENTER);
	playerBulletGraphic = initSpriteStatic(app.gameplaySprites, 20, 0, 2, 1, SC_CENTER);
	scrapGraphic1 = initSpriteStatic(app.gameplaySprites, 16, 17, 1, 1, SC_CENTER);
	scrapGraphic2 = initSpriteStatic(app.gameplaySprites, 16, 18, 1, 1, SC_CENTER);
	scrapGraphic3 = initSpriteStatic(app.gameplaySprites, 16, 19, 1, 1, SC_CENTER);
	scrapGraphic4 = initSpriteStatic(app.gameplaySprites, 16, 20, 1, 1, SC_CENTER);
	powerupBaseGraphic = initSpriteStatic(app.gameplaySprites, 8, 14, 2, 2, SC_CENTER);
	powerupLetterNGraphic = initSpriteStatic(app.gameplaySprites, 0, 14, 2, 2, SC_CENTER);
	powerupLetterEGraphic = initSpriteStatic(app.gameplaySprites, 2, 14, 2, 2, SC_CENTER);
	powerupLetterBGraphic = initSpriteStatic(app.gameplaySprites, 4, 14, 2, 2, SC_CENTER);
	powerupLetterSGraphic = initSpriteStatic(app.gameplaySprites, 6, 14, 2, 2, SC_CENTER);
}

void drawMainMenuUI(void) {
	//use title as a check to make sure the necessary sprites haven't been deleted by going into the game or exiting the app
	if (title) {
		drawBackground();

		blitSpriteStatic(title, SCREEN_WIDTH * 0.5, 30);

		drawWidgets("title");
	}
}

void drawHowToPlayUI(void) {
	drawBackground();

	drawTextDropShadow("Move:", LEFT_X, 40, PALETTE_LIGHT_BLUE, TAH_CENTER, NULL, PALETTE_BLACK, 1);
	blitSpriteStatic(leftStickGraphic, LEFT_X - 60, 70);
	blitSpriteStatic(dpadGraphic, LEFT_X - 40, 70);
	for(int i = -20; i < 60; i += 20)
		blitSpriteStatic(keyBackgroundGraphic, LEFT_X + i + 4, 70);
	drawTextDropShadow("W A S D", LEFT_X - 20, 70 - 6, PALETTE_WHITE, TAH_LEFT, NULL, PALETTE_BLACK, 1);

	drawTextDropShadow("Aim:", RIGHT_X, 40, PALETTE_LIGHT_BLUE, TAH_CENTER, NULL, PALETTE_BLACK, 1);
	blitSpriteStatic(rightStickGraphic, RIGHT_X - 10, 70);
	blitSpriteStatic(mouseGraphic, RIGHT_X + 10, 70);

	drawTextDropShadow("Fire bullets:", LEFT_X, 100, PALETTE_LIGHT_BLUE, TAH_CENTER, NULL, PALETTE_BLACK, 1);
	blitSpriteStatic(rightShoulderButtonGraphic, LEFT_X - 20, 130);
	blitSpriteStatic(bottomFaceButtonGraphic, LEFT_X, 130);
	blitSpriteStatic(leftMouseButtonGraphic, LEFT_X + 20, 130);
	blitSpriteStatic(playerShipGraphic, LEFT_X - 30, 170);
	blitSpriteStatic(playerBulletGraphic, LEFT_X + 30, 170);

	drawTextDropShadow("Avoid crates, enemies,", RIGHT_X, 100, PALETTE_LIGHT_BLUE, TAH_CENTER, NULL, PALETTE_BLACK, 1);
	drawTextDropShadow("and enemy bullets.", RIGHT_X, 120, PALETTE_LIGHT_BLUE, TAH_CENTER, NULL, PALETTE_BLACK, 1);
	blitSpriteStatic(crateGraphic, RIGHT_X - 60, 170);
	blitSpriteStatic(enemyShipGraphic, RIGHT_X, 170);
	blitSpriteStatic(enemyBulletGraphic, RIGHT_X + 60, 170);

	drawTextDropShadow("Collect scrap", LEFT_X, 210, PALETTE_LIGHT_BLUE, TAH_CENTER, NULL, PALETTE_BLACK, 1);
	drawTextDropShadow("for high score.", LEFT_X, 230, PALETTE_LIGHT_BLUE, TAH_CENTER, NULL, PALETTE_BLACK, 1);
	blitSpriteStatic(scrapGraphic1, LEFT_X - 30, 260);
	blitSpriteStatic(scrapGraphic2, LEFT_X - 10, 260);
	blitSpriteStatic(scrapGraphic3, LEFT_X + 10, 260);
	blitSpriteStatic(scrapGraphic4, LEFT_X + 30, 260);

	drawTextDropShadow("Collect powerups for a", RIGHT_X, 210, PALETTE_LIGHT_BLUE, TAH_CENTER, NULL, PALETTE_BLACK, 1);
	drawTextDropShadow("new weapon and a bonus.", RIGHT_X, 230, PALETTE_LIGHT_BLUE, TAH_CENTER, NULL, PALETTE_BLACK, 1);
	blitSpriteStatic(powerupBaseGraphic, RIGHT_X - 60, 265);
	blitSpriteStatic(powerupLetterNGraphic, RIGHT_X - 60, 265);
	blitSpriteStatic(powerupBaseGraphic, RIGHT_X - 20, 265);
	blitSpriteStatic(powerupLetterEGraphic, RIGHT_X - 20, 265);
	blitSpriteStatic(powerupBaseGraphic, RIGHT_X + 20, 265);
	blitSpriteStatic(powerupLetterBGraphic, RIGHT_X + 20, 265);
	blitSpriteStatic(powerupBaseGraphic, RIGHT_X + 60, 265);
	blitSpriteStatic(powerupLetterSGraphic, RIGHT_X + 60, 265);

	//draw back button
	drawWidgets("back");
}

void drawHighscoresUI(void) {
	drawBackground();

	//draw highscores without displaying the user's latest score
	drawHighscores(false);

	//draw widgets
	drawWidgets("back");
}

void drawOptionsUI(void) {
	drawBackground();

	//draw widgets
	drawWidgets("options");
}

void drawCreditsUI(void) {
	drawBackground();

	//draw credits text
	drawTextDropShadow("Programming, graphics, sound, and music by:", SCREEN_WIDTH * 0.5, 120, PALETTE_LIGHT_BLUE, TAH_CENTER, NULL, PALETTE_BLACK, 1);
	drawTextDropShadow("Evan Evers", SCREEN_WIDTH * 0.5, 150, PALETTE_WHITE, TAH_CENTER, NULL, PALETTE_BLACK, 1);
	drawTextDropShadow("Special thanks to:", SCREEN_WIDTH * 0.5, 180, PALETTE_LIGHT_BLUE, TAH_CENTER, NULL, PALETTE_BLACK, 1);
	drawTextDropShadow("The SDL development team", SCREEN_WIDTH * 0.5, 210, PALETTE_WHITE, TAH_CENTER, NULL, PALETTE_BLACK, 1);
	drawTextDropShadow("Dave Gamble", SCREEN_WIDTH * 0.5, 240, PALETTE_WHITE, TAH_CENTER, NULL, PALETTE_BLACK, 1);

	//draw back button
	drawWidgets("back");
}

void deleteMainMenuUI(void) {
	//having to NULL all these pointers after deallocating them is not necessarily necessary, but it's good practice
	//also this a great example of why I need a SpriteStaticDelete() function, to consolidate the free() call and NULLing the pointer
	free(title);
	title = NULL;

	free(leftStickGraphic);
	leftStickGraphic = NULL;
	free(dpadGraphic);
	dpadGraphic = NULL;
	free(keyBackgroundGraphic);
	keyBackgroundGraphic = NULL;
	free(rightStickGraphic);
	rightStickGraphic = NULL;
	free(mouseGraphic);
	mouseGraphic = NULL;
	free(leftMouseButtonGraphic);
	leftMouseButtonGraphic = NULL;
	free(rightShoulderButtonGraphic);
	rightShoulderButtonGraphic = NULL;
	free(bottomFaceButtonGraphic);
	bottomFaceButtonGraphic = NULL;

	free(crateGraphic);
	crateGraphic = NULL;
	free(enemyShipGraphic);
	enemyShipGraphic = NULL;
	free(enemyBulletGraphic);
	enemyBulletGraphic = NULL;
	free(playerShipGraphic);
	playerShipGraphic = NULL;
	free(playerBulletGraphic);
	playerBulletGraphic = NULL;
	free(scrapGraphic1);
	scrapGraphic1 = NULL;
	free(scrapGraphic2);
	scrapGraphic2 = NULL;
	free(scrapGraphic3);
	scrapGraphic3 = NULL;
	free(scrapGraphic4);
	scrapGraphic4 = NULL;
	free(powerupBaseGraphic);
	powerupBaseGraphic = NULL;
	free(powerupLetterNGraphic);
	powerupLetterNGraphic = NULL;
	free(powerupLetterEGraphic);
	powerupLetterEGraphic = NULL;
	free(powerupLetterBGraphic);
	powerupLetterBGraphic = NULL;
	free(powerupLetterSGraphic);
	powerupLetterSGraphic = NULL;
}