#include "common.h"

#include "background.h"
#include "stage.h"

extern App app;
extern Stage stage;
extern Background background;

void initBackground(void);
void drawBackground(void);
void deleteBackground(void);

SDL_Rect screenRect = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };	//a rectangle covering the screen for drawing backgrounds to

void initBackground(void) {
	//initialize background timers
	background.backgroundFlashRedTimer = END_OF_FLASH * 3;
	background.backgroundFlashWhiteTimer = END_OF_FLASH * 3;

	//initalize planet
	background.planet = initParticle(initSpriteAnimated(app.gameplaySprites, 8, 0, 5, 5, SC_CENTER, 1, 0, 0, AL_ONESHOT), randFloat(SCREEN_WIDTH), randFloat(SCREEN_HEIGHT), 0, 0, 0, 1, NULL, NULL);

	//init 3 layers of stars
	//closer ones blink faster, further ones blink slower
	for (int i = 0; i < NUM_BACKGROUND_STARS_L1; ++i) {
		background.starsLayer1[i] = initParticle(initSpriteAnimated(app.gameplaySprites, 26, randInt(6), 1, 1, SC_CENTER, 4, 0, randFloatRange(0.025, 0.05), AL_LOOP), randFloat(SCREEN_WIDTH), randFloat(SCREEN_HEIGHT), 0, 0, 0, 1, NULL, NULL);
	}
	for (int i = 0; i < NUM_BACKGROUND_STARS_L2; ++i) {
		background.starsLayer2[i] = initParticle(initSpriteAnimated(app.gameplaySprites, 26, randInt(6), 1, 1, SC_CENTER, 4, 0, randFloatRange(0.01, 0.025), AL_LOOP), randFloat(SCREEN_WIDTH), randFloat(SCREEN_HEIGHT), 0, 0, 0, 1, NULL, NULL);
	}
	for (int i = 0; i < NUM_BACKGROUND_STARS_L3; ++i) {
		background.starsLayer3[i] = initParticle(initSpriteAnimated(app.gameplaySprites, 26, randInt(6), 1, 1, SC_CENTER, 4, 0, randFloatRange(0.005, 0.01), AL_LOOP), randFloat(SCREEN_WIDTH), randFloat(SCREEN_HEIGHT), 0, 0, 0, 1, NULL, NULL);
	}
}

//instead of making particle draw functions, the background is drawn with one draw function for greater efficiency and proper layering
//stars are drawn with less alpha the further back they are
void drawBackground(void) {
	//draw black background
	//draw a semitransparent black rectangle over the screen
	//stuff like this is why I need a rectangle drawing function
	SDL_SetRenderDrawBlendMode(app.renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(app.renderer, PALETTE_BLACK.r, PALETTE_BLACK.g, PALETTE_BLACK.b, PALETTE_BLACK.a);
	SDL_RenderFillRect(app.renderer, &screenRect);

	//draw red flash when player is hit
	//only do this if alpha is greater than 0
	if (background.backgroundFlashRedTimer < END_OF_FLASH * 3) {
		SDL_SetRenderDrawBlendMode(app.renderer, SDL_BLENDMODE_BLEND);
		SDL_SetRenderDrawColor(app.renderer, PALETTE_RED.r, PALETTE_RED.g, PALETTE_RED.b, (int)(255 * (float)((END_OF_FLASH * 3) - background.backgroundFlashRedTimer) / (float)(END_OF_FLASH * 3)));
		SDL_RenderFillRect(app.renderer, &screenRect);
	}

	//draw white flash when player collects a powerup
	//only do this if alpha is greater than 0
	if (background.backgroundFlashWhiteTimer < END_OF_FLASH * 3) {
		SDL_SetRenderDrawBlendMode(app.renderer, SDL_BLENDMODE_BLEND);
		SDL_SetRenderDrawColor(app.renderer, PALETTE_WHITE.r, PALETTE_WHITE.g, PALETTE_WHITE.b, (int)(63 * (float)((END_OF_FLASH * 3) - background.backgroundFlashWhiteTimer) / (float)(END_OF_FLASH * 3)));
		SDL_RenderFillRect(app.renderer, &screenRect);
	}

	//draw stars
	setTextureRGBA(app.gameplaySprites->texture, 255, 255, 255, 63);
	for (int i = 0; i < NUM_BACKGROUND_STARS_L3; ++i) {
		blitAndUpdateSpriteAnimated(background.starsLayer3[i]->sprite, background.starsLayer3[i]->x, background.starsLayer3[i]->y);
	}
	setTextureRGBA(app.gameplaySprites->texture, 255, 255, 255, 127);
	for (int i = 0; i < NUM_BACKGROUND_STARS_L2; ++i) {
		blitAndUpdateSpriteAnimated(background.starsLayer2[i]->sprite, background.starsLayer2[i]->x, background.starsLayer2[i]->y);
	}
	setTextureRGBA(app.gameplaySprites->texture, 255, 255, 255, 255);
	for (int i = 0; i < NUM_BACKGROUND_STARS_L1; ++i) {
		blitAndUpdateSpriteAnimated(background.starsLayer1[i]->sprite, background.starsLayer1[i]->x, background.starsLayer1[i]->y);
	}

	//draw planet
	blitAndUpdateSpriteAnimated(background.planet->sprite, background.planet->x, background.planet->y);

	//update timers
	++background.backgroundFlashRedTimer;
	++background.backgroundFlashWhiteTimer;
}

void deleteBackground(void) {
	//sorry nothing
	//this used to have a reason for existing, but now it doesn't
	
	//particles automatically deallocated by particle delete function
}