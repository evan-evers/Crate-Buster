#include "common.h"

#include "background.h"
#include "draw.h"
#include "fonts.h"
#include "geometry.h"
#include "particles.h"
#include "player.h"
#include "powerups.h"
#include "scrap.h"
#include "sound.h"
#include "stage.h"

extern App app;
extern Player *player;
extern Stage stage;
extern Background background;
extern const int PLAYER_HP_MAX;
extern int timeSincePowerupCollected;

void initPowerup(int x, int y);

static const float POWERUP_MOVE_DISTANCE_SQUARED = 100 * 100;	//how close to the player the powerup must be to start moving (squared)
static const float POWERUP_ACCEL = 0.5;
static const float POWERUP_MAX_SPD = 5;
static const int HORZ_EDGE_DIST = 16;	//screenwrap var
static const int VERT_EDGE_DIST = 16;	//screenwrap var
static const float POWERUP_COLLECT_DISTANCE_SQUARED = 25 * 25;	//distance at which a piece of scrap is collected by the player, squared for an optimized calculation
static const int POWERUP_TEXT_OFFSET = 50;	//how far offset the collection info text should be from the center of the powerup
static const char * const POWERUP_INFO_TEXT_HP = "+HP!";
static const char * const POWERUP_INFO_TEXT_SCRAP = "+SCRAP!";
static char powerupInfoText[10];	//buffer that holds the text to be referred to by the powerup info text function
static SpriteStatic *powerupCell = NULL;
static SpriteStatic *powerupCellShine = NULL;

static void powerupCollectShockwaveUpdate(Particle *particle) {
	particle->x = player->x;
	particle->y = player->y;
}

static void powerupCollectShockwaveDraw(Particle *particle) {
	//draw if the animation isn't over yet (using extra frame trick to tell when animation is done)
	if (particle->sprite->currentFrame < particle->sprite->frames - 1) {
		//complex draw event for a dumb over-optimized sprite
		blitAndUpdateSpriteAnimatedEX(particle->sprite, particle->x, particle->y, particle->angle, NULL, SDL_FLIP_NONE);
		blitAndUpdateSpriteAnimatedEX(particle->sprite, particle->x + particle->sprite->w, particle->y, particle->angle, NULL, SDL_FLIP_HORIZONTAL);
		blitAndUpdateSpriteAnimatedEX(particle->sprite, particle->x, particle->y + particle->sprite->h, particle->angle, NULL, SDL_FLIP_VERTICAL);
		blitAndUpdateSpriteAnimatedEX(particle->sprite, particle->x + particle->sprite->w, particle->y + particle->sprite->h, particle->angle, NULL, SDL_FLIP_HORIZONTAL | SDL_FLIP_VERTICAL);
	}
	else
		particle->ttl = 0; 	//delete particle when animation is over
}

static void powerupInfoTextUpdate(Particle *particle) {
	particle->x = player->x;
	particle->y = player->y - POWERUP_TEXT_OFFSET;
}

static void powerupInfoTextDraw(Particle *particle) {
	//flash text
	if (particle->ttl % 20 > 5) {
		drawTextDropShadow(powerupInfoText, particle->x, particle->y, PALETTE_WHITE, TAH_CENTER, NULL, PALETTE_BLACK, 1);
	}

	--particle->ttl;
}

//update function for powerups
static void powerupUpdate(Particle *powerup) {
	//accelerate towards the player if closeby & player isn't dead
	if (distanceSquared(powerup->x, powerup->y, player->x, player->y) < POWERUP_MOVE_DISTANCE_SQUARED && player != PS_DESTROYED) {
		if (player->x < powerup->x)
			powerup->deltaX = MAX(powerup->deltaX - POWERUP_ACCEL, -POWERUP_MAX_SPD);
		else
			powerup->deltaX = MIN(powerup->deltaX + POWERUP_ACCEL, POWERUP_MAX_SPD);
		if (player->y < powerup->y)
			powerup->deltaY = MAX(powerup->deltaY - POWERUP_ACCEL, -POWERUP_MAX_SPD);
		else
			powerup->deltaY = MIN(powerup->deltaY + POWERUP_ACCEL, POWERUP_MAX_SPD);
	}
	else {
		//if not closeby, slow down
		if (powerup->deltaX < 0)
			powerup->deltaX = MIN(powerup->deltaX + POWERUP_ACCEL, 0);
		else
			powerup->deltaX = MAX(powerup->deltaX - POWERUP_ACCEL, 0);
		if (powerup->deltaY < 0)
			powerup->deltaY = MIN(powerup->deltaY + POWERUP_ACCEL, 0);
		else
			powerup->deltaY = MAX(powerup->deltaY - POWERUP_ACCEL, 0);
	}

	//move powerup
	powerup->x += powerup->deltaX;
	powerup->y += powerup->deltaY;

	//screenwrap
	if (powerup->x < -HORZ_EDGE_DIST)
		powerup->x = SCREEN_WIDTH + HORZ_EDGE_DIST;
	if (powerup->x > SCREEN_WIDTH + HORZ_EDGE_DIST)
		powerup->x = -HORZ_EDGE_DIST;
	if (powerup->y < -VERT_EDGE_DIST)
		powerup->y = SCREEN_HEIGHT + VERT_EDGE_DIST;
	if (powerup->y > SCREEN_HEIGHT + VERT_EDGE_DIST)
		powerup->y = -VERT_EDGE_DIST;

	//if close enough to the player & player isn't dead, increment score and delete this projectile
	if (distanceSquared(powerup->x, powerup->y, player->x, player->y) < POWERUP_COLLECT_DISTANCE_SQUARED && player != PS_DESTROYED) {
		//collection particle
		initParticle(initSpriteAnimated(app.gameplaySprites, 0, 16, 2, 2, SC_BOTTOM_RIGHT, 5, 0, 0.05, AL_ONESHOT), player->x, player->y, 0, 0, 0, 1, powerupCollectShockwaveUpdate, powerupCollectShockwaveDraw);
		playSound(SFX_POWER_UP, SC_ANY, false, powerup->x / SCREEN_HEIGHT * 255);

		//set player to do powerup collection flash
		timeSincePowerupCollected = 0;

		//set background to do powerup collection flash
		background.backgroundFlashWhiteTimer = 0;

		//change player weapon
		switch (powerup->sprite->srcX) {
		case(0):
			player->weaponType = BT_NORMAL;
			break;
		case(32):
			player->weaponType = BT_ERRATIC;
			break;
		case(64):
			player->weaponType = BT_BOUNCER;
			break;
		case(96):
			player->weaponType = BT_SHOTGUN;
			break;
		}

		//give player hp if they need it; otherwise, give a bunch of scrap.
		if (player->hp < PLAYER_HP_MAX) {
			++player->hp;
			
			STRNCPY(powerupInfoText, POWERUP_INFO_TEXT_HP, 6);
		}
		else {
			//if player's hp is full, give scrap
			for (int i = 0; i < 20; ++i)
				initScrap(player->x + randFloatRange(-40, 40), player->y + randFloatRange(-40, 40));

			STRNCPY(powerupInfoText, POWERUP_INFO_TEXT_SCRAP, 9);
		}

		//create info text as a particle
		initParticle(NULL, player->x, player->y - POWERUP_TEXT_OFFSET, 0, 0, 0, FPS, powerupInfoTextUpdate, powerupInfoTextDraw);

		powerup->ttl = 0;
	}
}

//draw function for a powerup
static void powerupDraw(Particle *powerup) {
	blitSpriteStatic(powerupCell, powerup->x, powerup->y);
	setTextureRGBA(powerup->sprite->atlas->texture, 255, 255, 255, sin((float)(stage.timer) / 5) * 127 + 127);	//blinking effect
	blitSpriteStatic(powerupCellShine, powerup->x, powerup->y);
	setTextureRGBA(powerup->sprite->atlas->texture, 255, 255, 255, 255);
	blitAndUpdateSpriteAnimatedEX(powerup->sprite, powerup->x, powerup->y, powerup->angle, NULL, SDL_FLIP_NONE);
}

//initializes a new powerup
void initPowerup(int x, int y) {
	//initialize particle as a type that the player doesn't already have
	BulletType type;

	do {
		switch (rand() % 4) {
			case(0):
				type = BT_NORMAL;
				break;
			case(1):
				type = BT_ERRATIC;
				break;
			case(2):
				type = BT_BOUNCER;
				break;
			case(3):
				type = BT_SHOTGUN;
				break;
			default:
				type = BT_NORMAL;
		}
	} while (type == player->weaponType);

	//ensure powerup spawns within the boundaries of the stage
	if (x < powerupCell->w * 0.5)
		x = powerupCell->w * 0.5;
	if (x > SCREEN_WIDTH - powerupCell->w * 0.5)
		x = SCREEN_WIDTH - powerupCell->w * 0.5;
	if (y < powerupCell->h * 0.5)
		y = powerupCell->h * 0.5;
	if (y > SCREEN_HEIGHT - powerupCell->h * 0.5)
		y = SCREEN_HEIGHT - powerupCell->h * 0.5;

	//initialize powerup
	switch (type) {
		case(BT_NORMAL):
			initParticle(initSpriteAnimated(app.gameplaySprites, 0, 14, 2, 2, SC_CENTER, 1, 0, 0, AL_ONESHOT), x, y, 0, 0, 0, 1, powerupUpdate, powerupDraw);
			break;
		case(BT_ERRATIC):
			initParticle(initSpriteAnimated(app.gameplaySprites, 2, 14, 2, 2, SC_CENTER, 1, 0, 0, AL_ONESHOT), x, y, 0, 0, 0, 1, powerupUpdate, powerupDraw);
			break;
		case(BT_BOUNCER):
			initParticle(initSpriteAnimated(app.gameplaySprites, 4, 14, 2, 2, SC_CENTER, 1, 0, 0, AL_ONESHOT), x, y, 0, 0, 0, 1, powerupUpdate, powerupDraw);
			break;
		case(BT_SHOTGUN):
			initParticle(initSpriteAnimated(app.gameplaySprites, 6, 14, 2, 2, SC_CENTER, 1, 0, 0, AL_ONESHOT), x, y, 0, 0, 0, 1, powerupUpdate, powerupDraw);
			break;
		default:
			initParticle(initSpriteAnimated(app.gameplaySprites, 0, 14, 2, 2, SC_CENTER, 1, 0, 0, AL_ONESHOT), x, y, 0, 0, 0, 1, powerupUpdate, powerupDraw);
	}
}

//initialize common sprites
void initPowerups(void) {
	powerupCell = initSpriteStatic(app.gameplaySprites, 8, 14, 2, 2, SC_CENTER);
	powerupCellShine = initSpriteStatic(app.gameplaySprites, 10, 14, 2, 2, SC_CENTER);
}

//deletes common sprites
void deletePowerups(void) {
	free(powerupCell);
	free(powerupCellShine);
}

//no destructor needed; since a powerup is a particle, when its ttl is set to 0, it gets deleted by updateParticles when called in the stage update function