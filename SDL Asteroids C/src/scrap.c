#include "common.h"

#include "draw.h"
#include "geometry.h"
#include "particles.h"
#include "player.h"
#include "scrap.h"
#include "stage.h"

extern App app;
extern Player *player;
extern Stage stage;

void initScrap(int x, int y);

static const float SCRAP_ACCEL = 0.3;
static const float SCRAP_MAX_SPD = 5;
static const int SCRAP_VALUE = 5;
static const int HORZ_EDGE_DIST = 8;	//screenwrap var
static const int VERT_EDGE_DIST = 8;	//screenwrap var
static const float SCRAP_COLLECT_DISTANCE_SQUARED = 25 * 25;	//distance at which a piece of scrap is collected by the player, squared for an optimized calculation

//scrap collection particle currently not being used b/c isn't just kind of visually noisy

//scrap collection particle doesn't need an update function

//draw function for scrap collection particle
//static void scrapCollectFlashDraw(Particle *particle) {
//	//draw if the animation isn't over yet (using extra frame trick to tell when animation is done)
//	if (particle->sprite->currentFrame < particle->sprite->frames - 1) {
//		setTextureRGBA(particle->sprite->atlas->texture, 255, 255, 255, 127);
//		blitAndUpdateSpriteAnimatedEX(particle->sprite, particle->x, particle->y, particle->angle, NULL, SDL_FLIP_NONE);
//		setTextureRGBA(particle->sprite->atlas->texture, 255, 255, 255, 255);
//	}
//	else
//		particle->ttl = 0; 	//delete particle when animation is over
//}

//update function for scrap
static void scrapUpdate(Particle *scrap) {
	//accelerate towards the player
	if (player->x < scrap->x)
		scrap->deltaX = MAX(scrap->deltaX - SCRAP_ACCEL, -SCRAP_MAX_SPD);
	else
		scrap->deltaX = MIN(scrap->deltaX + SCRAP_ACCEL, SCRAP_MAX_SPD);
	if (player->y < scrap->y)
		scrap->deltaY = MAX(scrap->deltaY - SCRAP_ACCEL, -SCRAP_MAX_SPD);
	else
		scrap->deltaY = MIN(scrap->deltaY + SCRAP_ACCEL, SCRAP_MAX_SPD);

	//move scrap
	scrap->x += scrap->deltaX;
	scrap->y += scrap->deltaY;

	//screenwrap
	if (scrap->x < -HORZ_EDGE_DIST)
		scrap->x = SCREEN_WIDTH + HORZ_EDGE_DIST;
	if (scrap->x > SCREEN_WIDTH + HORZ_EDGE_DIST)
		scrap->x = -HORZ_EDGE_DIST;
	if (scrap->y < -VERT_EDGE_DIST)
		scrap->y = SCREEN_HEIGHT + VERT_EDGE_DIST;
	if (scrap->y > SCREEN_HEIGHT + VERT_EDGE_DIST)
		scrap->y = -VERT_EDGE_DIST;

	//if close enough to the player, increment score and delete this projectile
	if (distanceSquared(scrap->x, scrap->y, player->x, player->y) < SCRAP_COLLECT_DISTANCE_SQUARED && player->state != PS_DESTROYED) {
		stage.score += SCRAP_VALUE;

		//initParticle(initSpriteAnimated(app.gameplaySprites, 16, 21, 1, 1, SC_CENTER, 5, 0, 0.15, AL_ONESHOT), scrap->x, scrap->y, 0, 0, 0, 1, NULL, scrapCollectFlashDraw);

		scrap->ttl = 0;
	}
}

//draw function for scrap
static void scrapDraw(Particle *scrap) {
	blitAndUpdateSpriteAnimatedEX(scrap->sprite, scrap->x, scrap->y, scrap->angle, NULL, SDL_FLIP_NONE);
}

//initializes a new piece of scrap
void initScrap(int x, int y) {
	//initialize particle with a random sprite out of the four
	initParticle(initSpriteAnimated(app.gameplaySprites, 16, 17 + rand() % 4, 1, 1, SC_CENTER, 4, 0, randFloatRange(0.1, 0.5), AL_LOOP), x, y, 0, 0, randFloat(359.99999999999999), 1, scrapUpdate, scrapDraw);
}

//no destructor needed; since a piece of scrap is a particle, when its ttl is set to 0, it gets deleted by updateParticles when called in the stage update function