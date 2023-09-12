#include "common.h"

#include "player.h"

#include "background.h"
#include "bullets.h"
#include "colliders.h"
#include "crates.h"
#include "draw.h"
#include "fonts.h"
#include "geometry.h"
#include "particles.h"
#include "sound.h"
#include "stage.h"
#include "utility.h"

extern App app;
extern InputManager input;
extern Player *player;
extern Stage stage;
extern Background background;

static void psNormal();
static void psDashing();
void updatePlayer();
static void checkHitCrates(void);
static void checkHitEnemies(void);
void drawPlayer();
static void mfUpdate(Particle *particle);
static void mfDraw(Particle *particle);
void initPlayer(int x, int y);
void deletePlayer();

static const float PLAYER_SPEED_MAX = 3;
static const float PLAYER_SPEED_ACCEL = 0.075;
static const float PLAYER_SPEED_DECCEL = 0.025;
static const int DASH_TIME_MAX = 20;	//maximum amount of time spent in dash state
static const float DASH_SPEED_DECAY = 0.30;	//speed decay per frame in dash state
static const float DASH_SPEED_MAX = 10;	//maximum speed at start of dash state
static const float ACCEL = 0.25;	//acceleration
static const float DECCEL = 0.125;	//decelleration
static const int DEATH_TIMER_MAX = 120;	//time until respawning
const int PLAYER_HP_MAX = 3;	//not static, as this is referred to by powerups.h
static float horzEdgeDist = 48 * SCREENWRAP_MARGIN;	//screenwrap margins
static float vertEdgeDist = 48 * SCREENWRAP_MARGIN;
static bool showShipFlame = false;	//keeps track of whether or not the spaceship's flame should be shown

static int deathTimer = 0;	//keeps track of time until respawn
//muzzle flash particles
static Particle *mfNormal = NULL;
static Particle *mfErratic = NULL;
static Particle *mfBouncer = NULL;
static Particle *mfShotgun = NULL;
//powerup flash vars
static SpriteStatic *powerupFlash = NULL;
int timeSincePowerupCollected = END_OF_FLASH;

//normal player state
static void psNormal() {
	//movement

	//update spaceship angle
	if (input.lastControllerType == LCT_KEYBOARD_AND_MOUSE) {
		//angle spaceship towards mouse
		float targetAngle = atan2((input.mouse.y - player->y), (input.mouse.x - player->x)) * RADIANS_TO_DEGREES;
		player->angle = targetAngle;
	}
	else {
		//angle spaceship in same direction as right stick
		//if right stick isn't being held, angle the player with the left stick
		if(fabs(input.rightLR) > 0 || fabs(input.rightUD) > 0)
			player->angle = atan2(input.rightUD, input.rightLR) * RADIANS_TO_DEGREES;
		else if (fabs(input.leftLR) > 0 || fabs(input.leftUD) > 0)
			player->angle = atan2(input.leftUD, input.leftLR) * RADIANS_TO_DEGREES;
	}

	//calculate intended player movement
	if (fabs(input.leftLR) > 0 || fabs(input.leftUD) > 0) {
		player->dirVector.x = input.leftLR;
		player->dirVector.y = input.leftUD;
		showShipFlame = true;	//for aesthetics
	}
	else {
		player->dirVector.x = 0;
		player->dirVector.y = 0;
		showShipFlame = false;
	}
	normalize(&player->dirVector);

	//update player's actual movement according to intent
	//this is my old approach function; should probably turn it into a function and put it in utility.c/h
	//use accel var to update momentum if player is holding a direction; use deccel var if player isn't holding a direction
	if (fabs(input.leftLR) > 0 || fabs(input.leftUD) > 0) {
		approach(&player->momentumVector.x, player->dirVector.x, PLAYER_SPEED_ACCEL);
		approach(&player->momentumVector.y, player->dirVector.y, PLAYER_SPEED_ACCEL);
	}
	else {
		approach(&player->momentumVector.x, player->dirVector.x, PLAYER_SPEED_DECCEL);
		approach(&player->momentumVector.y, player->dirVector.y, PLAYER_SPEED_DECCEL);
	}

	////update momentum
	//if (fabs(input.leftLR) > 0 || fabs(input.leftUD) > 0) {
	//	player->speed = MIN(PLAYER_SPEED_MAX, player->speed + ACCEL);
	//}
	//else {
	//	player->speed = MAX(0, player->speed - DECCEL);
	//}

	//update position
	player->x += player->momentumVector.x * PLAYER_SPEED_MAX;
	player->y += player->momentumVector.y * PLAYER_SPEED_MAX;

	//screenwrap
	if (player->x < -horzEdgeDist)
		player->x = SCREEN_WIDTH + horzEdgeDist;
	if (player->x > SCREEN_WIDTH + horzEdgeDist)
		player->x = -horzEdgeDist;
	if (player->y < -vertEdgeDist)
		player->y = SCREEN_HEIGHT + vertEdgeDist;
	if (player->y > SCREEN_HEIGHT + vertEdgeDist)
		player->y = -vertEdgeDist;

	//update collider
	updateCollider(player->collider, player->x, player->y, player->angle * DEGREES_TO_RADIANS, -1, -1);

	//check if the player's hitbox intersected with any crates; if so, decrement player and box hp
	checkHitCrates();

	//check if player touches an enemy
	checkHitEnemies();

	//decrement invincibility frames
	--player->iFrames;

	//firing guns (updates reload)
	if (--player->reload <= 0 && input.fire > 0 && stage.state == SS_GAMEPLAY) {
		firePlayerBullet();

		//set muzzle flash animation and fire sound to play
		switch (player->weaponType) {
			case(BT_NORMAL):
				mfNormal->sprite->currentFrame = 0;
				playSoundIsolated(SFX_SHOT_FIRE_1, SC_PLAYER_FIRE, false, player->x / SCREEN_WIDTH * 255.0);
				break;
			case(BT_ERRATIC):
				mfErratic->sprite->currentFrame = 0;
				playSoundIsolated(SFX_SHOT_FIRE_2, SC_PLAYER_FIRE, false, player->x / SCREEN_WIDTH * 255.0);
				break;
			case(BT_BOUNCER):
				mfBouncer->sprite->currentFrame = 0;
				playSoundIsolated(SFX_SHOT_FIRE_3, SC_PLAYER_FIRE, false, player->x / SCREEN_WIDTH * 255.0);
				break;
			case(BT_SHOTGUN):
				mfShotgun->sprite->currentFrame = 0;
				playSoundIsolated(SFX_SHOT_FIRE_4, SC_PLAYER_FIRE, false, player->x / SCREEN_WIDTH * 255.0);
				break;
		}

		//reset buffer (to prevent firing after button was released)
		input.fire = 0;
	}

	//state changes
	//if (input.dashPressed > 0) {
	//	//if player isn't inputting a direction, dash towards the cursor
	//	if (input.leftLR == 0 && input.leftUD == 0) {
	//		player->dirVector.x = cos(player->angle * DEGREES_TO_RADIANS);
	//		player->dirVector.y = sin(player->angle * DEGREES_TO_RADIANS);
	//	}

	//	player->speed = DASH_SPEED_MAX;
	//	player->dashTimer = DASH_TIME_MAX;
	//	player->state = PS_DASHING;

	//	//reset buffer
	//	input.dashPressed = 0;
	//}

	//kill player when HP is 0
	if (player->hp <= 0) {
		//death explosion
		initParticle(initSpriteAnimated(app.gameplaySprites, 0, 18, 4, 4, SC_CENTER, 5, 0, 0.3, AL_ONESHOT), player->x, player->y, 0, 0, (float)(rand() % 4) * 90, 1, NULL, explosionDraw);
		playSound(SFX_PLAYER_KILL, SC_PLAYER, false, player->x / SCREEN_WIDTH * 255.0);

		player->state = PS_DESTROYED;
	}
}

//dashing player state (currently unused)
static void psDashing() {
	//movement

	//update spaceship angle
	if (input.lastControllerType == LCT_KEYBOARD_AND_MOUSE) {
		//angle spaceship towards mouse
		float targetAngle = atan2((input.mouse.y - player->y), (input.mouse.x - player->x)) * RADIANS_TO_DEGREES;
		player->angle = targetAngle;
	}
	else {
		//angle spaceship in same direction as right stick
		player->angle = atan2(input.rightUD, input.rightLR) * RADIANS_TO_DEGREES;
	}

	//decrement speed
	player->speed -= DASH_SPEED_DECAY;

	//update player movement
	player->x += player->dirVector.x * player->speed;
	player->y += player->dirVector.y * player->speed;

	//screenwrap
	if (player->x < -horzEdgeDist)
		player->x = SCREEN_WIDTH + horzEdgeDist;
	if (player->x > SCREEN_WIDTH + horzEdgeDist)
		player->x = -horzEdgeDist;
	if (player->y < -vertEdgeDist)
		player->y = SCREEN_HEIGHT + vertEdgeDist;
	if (player->y > SCREEN_HEIGHT + vertEdgeDist)
		player->y = -vertEdgeDist;

	//update collider
	updateCollider(player->collider, player->x, player->y, player->angle, -1, -1);

	//check if the player's hitbox intersected with any crates; if so, decrement player and box hp
	checkHitCrates();

	//check if player touches an enemy
	checkHitEnemies();

	//decrement invincibility frames
	--player->iFrames;

	//state changes

	//decrement dash timer and transition to normal movement when dash is over
	if (--player->dashTimer == 0) {
		player->state = PS_NORMAL;
	}

	//kill player when HP is 0
	if (player->hp <= 0) {
		deathTimer = DEATH_TIMER_MAX;
		player->state = PS_DESTROYED;
	}
}

//destroyed player state
static void psDestroyed() {
	//do nothing. you're dead.
}

//update player every game step
void updatePlayer() {
	switch (player->state) {
		case(PS_NORMAL):
			psNormal(player);
		break;
		case(PS_DASHING):
			psDashing(player);
		break;
		case(PS_DESTROYED):
			psDestroyed(player);
			break;
		default:
			psNormal(player);
	}

	//progress powerup flash
	++timeSincePowerupCollected;
}

//check if the player's hit a crate; hurt player and destroy crate if so.
static void checkHitCrates(void) {
	Crate *crate = stage.crateHead;

	while (crate != NULL) {
		if (checkIntersection(player->collider, crate->collider)) {
			//decrement player HP, but only for the first crate they touch
			if (player->iFrames <= 0) {
				player->iFrames = PLAYER_I_FRAMES_MAX;	//give i-frames
				--player->hp;

				//set background to do hurt flash
				background.backgroundFlashRedTimer = 0;

				playSound(SFX_PLAYER_HIT, SC_PLAYER, false, player->x / SCREEN_WIDTH * 255.0);
			}
			crate->hp = 0;	//destroy crate
		}

		crate = crate->next;
	}
}

//check if the player's hit an enemy; hurt player and destroy enemy if so.
static void checkHitEnemies(void) {
	Enemy *enemy = stage.enemyHead;

	while (enemy != NULL) {
		if (checkIntersection(player->collider, enemy->collider)) {
			//decrement player HP, but only for the first enemy they touch
			if (player->iFrames <= 0) {
				player->iFrames = PLAYER_I_FRAMES_MAX;	//give i-frames
				--player->hp;

				//set background to do hurt flash
				background.backgroundFlashRedTimer = 0;

				playSound(SFX_PLAYER_HIT, SC_PLAYER, false, player->x / SCREEN_HEIGHT * 255);
			}
			enemy->hp = 0;	//destroy enemy
		}

		enemy = enemy->next;
	}
}

//draw player
void drawPlayer() {
	//only draw if player's not dead
	if (player != NULL && player->state != PS_DESTROYED) {
		if (player->iFrames <= 0 || player->iFrames % 10 > 5) {
			//draw as normal
			//draw sprite at center of player
			blitSpriteStaticEX(player->shipSprite, player->x, player->y, player->angle, NULL, SDL_FLIP_NONE, 255);
			//flame's just magic-numbered into place
			//flame's rotation origin is the center of the ship
			if (showShipFlame)
				blitAndUpdateSpriteAnimatedEX(player->shipFlame, player->x - 30, player->y, player->angle, &(SDL_Point){(player->shipFlame->w) * 0.5 + 30, player->shipFlame->h * 0.5}, SDL_FLIP_NONE, 255);
		}
		else {
			//blinking when i-frames are active
			//draw player with half-transparency
			setTextureRGBA(player->shipSprite->atlas->texture, 255, 255, 255, 127);		//since both of the player's sprite share the same atlas, i'm only calling this once
			blitSpriteStaticEX(player->shipSprite, player->x, player->y, player->angle, NULL, SDL_FLIP_NONE, 255);
			if (showShipFlame)
				blitAndUpdateSpriteAnimatedEX(player->shipFlame, player->x - 30, player->y, player->angle, &(SDL_Point){(player->shipFlame->w) * 0.5 + 30, player->shipFlame->h * 0.5}, SDL_FLIP_NONE, 255);
			setTextureRGBA(player->shipSprite->atlas->texture, 255, 255, 255, 255);	
		}

		//powerup flash
		setTextureRGBA(app.gameplaySprites->texture, 255, 255, 255, MAX((int)(255 * (float)(END_OF_FLASH - timeSincePowerupCollected) / (float)END_OF_FLASH), 0));
		blitSpriteStaticEX(powerupFlash, player->x, player->y, player->angle, NULL, SDL_FLIP_NONE);
		setTextureRGBA(app.gameplaySprites->texture, 255, 255, 255, 255);
	
		//only happens if app.debug = true
		displayCollider(app.renderer, &COLOR_RED, player->collider);
	}
}

//muzzle flash particle update function
static void mfUpdate(Particle *particle) {
	//only bother with expensive trig functions if the particle is visible
	if (particle->sprite->currentFrame != particle->sprite->frames - 1) {
		particle->x = player->x + cos(player->angle * DEGREES_TO_RADIANS) * (BULLET_OFFSET_PLAYER + 4);
		particle->y = player->y + sin(player->angle * DEGREES_TO_RADIANS) * (BULLET_OFFSET_PLAYER + 4);
	}
}

//muzzle flash particle draw function
static void mfDraw(Particle *particle) {
	//don't call if particle not visible
	if (particle->sprite->currentFrame != particle->sprite->frames - 1) {
		blitAndUpdateSpriteAnimatedEX(particle->sprite, particle->x, particle->y, player->angle, NULL, SDL_FLIP_NONE);
	}
}

//initialize player at pos (x,y)
void initPlayer(int x, int y) {
	player = malloc(sizeof(Player));
	memset(player, 0, sizeof(Player));
	player->x = x;
	player->y = y;
	player->angle = 0;
	player->speed = 0;
	player->state = PS_NORMAL;
	player->dirVector.x = 0;
	player->dirVector.y = 0;
	player->dashTimer = 0;
	player->reload = 0;
	player->hp = PLAYER_HP_MAX;	//3 hits before dying
	player->iFrames = 0;
	player->weaponType = BT_NORMAL;
	player->shipSprite = initSpriteStatic(app.gameplaySprites, 0, 11, 3, 3, SC_CENTER);
	player->shipFlame = initSpriteAnimated(app.gameplaySprites, 16, 16, 1, 1, SC_CENTER, 4, 0, 0.25, AL_LOOP);
	player->collider = initOBBCollider(player->shipSprite->w * 0.2, player->shipSprite->h * 0.2, (Vector2){ player->x, player->y }, player->angle);

	//initialize muzzle flash particles
	//particles initialized with an extra frame of animation to allow user to check when they've ended
	mfNormal = initParticle(initSpriteAnimated(app.gameplaySprites, 16, 2, 1, 1, SC_CENTER, 5, 5, 0.5, AL_ONESHOT), 0, 0, 0, 0, 0, 1, mfUpdate, mfDraw);
	mfErratic = initParticle(initSpriteAnimated(app.gameplaySprites, 16, 5, 1, 1, SC_CENTER, 5, 5, 0.5, AL_ONESHOT), 0, 0, 0, 0, 0, 1, mfUpdate, mfDraw);
	mfBouncer = initParticle(initSpriteAnimated(app.gameplaySprites, 16, 8, 1, 1, SC_CENTER, 5, 5, 0.5, AL_ONESHOT), 0, 0, 0, 0, 0, 1, mfUpdate, mfDraw);
	mfShotgun = initParticle(initSpriteAnimated(app.gameplaySprites, 16, 11, 1, 1, SC_CENTER, 5, 5, 0.5, AL_ONESHOT), 0, 0, 0, 0, 0, 1, mfUpdate, mfDraw);

	//initialize powerup flash
	powerupFlash = initSpriteStatic(app.gameplaySprites, 3, 11, 3, 3, SC_CENTER);
}

//resets certain player variables when the player respawns
void resetPlayer(int x, int y) {
	//reset important variables
	player->x = x;
	player->y = y;
	player->angle = 0;
	player->speed = 0;
	player->dirVector.x = 0;
	player->dirVector.y = 0;
	player->dashTimer = 0;
	player->reload = 0;
	player->hp = PLAYER_HP_MAX;	//3 hits before dying
	player->iFrames = 0;	//give the player respawn invulnerability in case they spawn on top of some crates
	player->weaponType = BT_NORMAL;

	player->state = PS_NORMAL;
}

//destruct player
void deletePlayer() {
	//free player struct stuff
	free(player->shipSprite);
	free(player->shipFlame);
	free(player->collider);
	free(player);
	player = NULL;

	//muzzle flash particles are automatically deallocated

	//delete powerup flash
	free(powerupFlash);
}