#include "common.h"

#include "player.h"

#include "bullets.h"
#include "colliders.h"
#include "crates.h"
#include "geometry.h"
#include "stage.h"

extern App app;
extern InputManager input;
extern Player *player;
extern Stage stage;

static void psNormal();
static void psDashing();
void updatePlayer();
static void checkHitCrates(void);
void drawPlayer();
void initPlayer(int x, int y);
void deletePlayer();

static const float PLAYER_SPEED_MAX = 3;
static const int DASH_TIME_MAX = 20;	//maximum amount of time spent in dash state
static const float DASH_SPEED_DECAY = 0.30;	//speed decay per frame in dash state
static const float DASH_SPEED_MAX = 10;	//maximum speed at start of dash state
static const float ACCEL = 0.25;	//acceleration
static const float DECCEL = 0.125;	//decelleration
static const float I_FRAMES_MAX = 60;	//number of invincibility frames after getting hit
static const int DEATH_TIMER_MAX = 120;	//time until respawning

static int deathTimer = 0;	//keeps track of time until respawn

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

	//calculate movement vector
	if (fabs(input.leftLR) > 0 || fabs(input.leftUD) > 0) {
		player->dirVector.x = input.leftLR;
		player->dirVector.y = input.leftUD;
	}
	normalize(&player->dirVector);

	//update momentum
	if (fabs(input.leftLR) > 0 || fabs(input.leftUD) > 0) {
		player->speed = MIN(PLAYER_SPEED_MAX, player->speed + ACCEL);
	}
	else {
		player->speed = MAX(0, player->speed - DECCEL);
	}

	//update player movement
	player->x += player->dirVector.x * player->speed;
	player->y += player->dirVector.y * player->speed;

	//clamp player to edges of screen
	if (player->x < 0)
		player->x = 0;
	if (player->x > SCREEN_WIDTH)
		player->x = SCREEN_WIDTH;
	if (player->y < 0)
		player->y = 0;
	if (player->y > SCREEN_HEIGHT)
		player->y = SCREEN_HEIGHT;

	//update collider
	updateCollider(player->collider, player->x, player->y, player->angle * DEGREES_TO_RADIANS, -1, -1);

	//check if the player's hitbox intersected with any crates; if so, decrement player and box hp
	checkHitCrates();

	//decrement invincibility frames
	--player->iFrames;

	//firing guns (updates reload)
	if (--player->reload <= 0 && input.fire > 0) {
		firePlayerBullet();

		//reset buffer (to prevent firing after button was released)
		input.fire = 0;
	}

	//just for fun for now: weapon switching
	if (input.pausePressed > 0) {
		++player->weaponType;

		if (player->weaponType >= WT_ENEMY) {
			player->weaponType = WT_NORMAL;
		}

		//reset buffer
		input.pausePressed = 0;
	}

	//state changes
	if (input.dashPressed > 0) {
		//if player isn't inputting a direction, dash towards the cursor
		if (input.leftLR == 0 && input.leftUD == 0) {
			player->dirVector.x = cos(player->angle * DEGREES_TO_RADIANS);
			player->dirVector.y = sin(player->angle * DEGREES_TO_RADIANS);
		}

		player->speed = DASH_SPEED_MAX;
		player->dashTimer = DASH_TIME_MAX;
		player->state = PS_DASHING;

		//reset buffer
		input.dashPressed = 0;
	}

	//kill player when HP is 0
	if (player->hp <= 0) {
		deathTimer = DEATH_TIMER_MAX;
		player->state = PS_DESTROYED;
	}
}

//dashing player state
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

	//clamp player to edges of screen
	if (player->x < 0)
		player->x = 0;
	if (player->x > SCREEN_WIDTH)
		player->x = SCREEN_WIDTH;
	if (player->y < 0)
		player->y = 0;
	if (player->y > SCREEN_HEIGHT)
		player->y = SCREEN_HEIGHT;

	//update collider
	updateCollider(player->collider, player->x, player->y, player->angle, -1, -1);

	//check if the player's hitbox intersected with any crates; if so, decrement player and box hp
	checkHitCrates();

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
	deathTimer--;

	//kill player when HP is 0
	if (deathTimer <= 0) {
		//reset important variables
		player->x = SCREEN_WIDTH * 0.5;
		player->y = SCREEN_HEIGHT * 0.5;
		player->angle = 0;
		player->speed = 0;
		player->dirVector.x = 0;
		player->dirVector.y = 0;
		player->dashTimer = 0;
		player->reload = 0;
		player->hp = 3;	//3 hits before dying
		player->iFrames = I_FRAMES_MAX;	//give the player respawn invulnerability in case they spawn on top of some crates
		player->weaponType = WT_NORMAL;

		player->state = PS_NORMAL;
	}
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

	printf("HP: %d, iFrames: %d\n", player->hp, player->iFrames);
}

static void checkHitCrates(void) {
	Crate *crate = stage.crateHead;
	bool alreadyHit = false;

	while (crate != NULL) {
		if (checkCollision(player->collider, crate->collider)) {
			//decrement player HP, but only for the first crate they touch
			if (!alreadyHit && player->iFrames <= 0) {
				player->iFrames = I_FRAMES_MAX;	//give i-frames
				--player->hp;
				alreadyHit = true;
			}
			crate->hp = 0;	//destroy crate
		}

		crate = crate->next;
	}
}

//draw player
void drawPlayer() {
	//only draw if player's not dead
	if (player->state != PS_DESTROYED) {
		//blinking when i-frames are active
		if (player->iFrames <= 0 || player->iFrames % 10 > 5) {
			//draw sprite at center of player
			blitSpriteEX(player->shipSprite, player->x - player->shipSprite->w * 0.5, player->y - player->shipSprite->h * 0.5, SC_TOP_LEFT, player->angle, NULL, SDL_FLIP_NONE, 255);
			//flame's just magic-numbered into place
			//flame's rotation origin is the center of the ship
			blitAndUpdateSpriteAnimatedEX(player->shipFlame, player->x - player->shipSprite->w * 0.5 - player->shipFlame->w, player->y - player->shipFlame->h * 0.5,
				SC_TOP_LEFT, player->angle, &(SDL_Point){(player->shipSprite->w) * 0.5 + player->shipFlame->w, player->shipFlame->h * 0.5}, SDL_FLIP_NONE, 255);
		}
		else {
			//draw player with half-transparency
			blitSpriteEX(player->shipSprite, player->x - player->shipSprite->w * 0.5, player->y - player->shipSprite->h * 0.5, SC_TOP_LEFT, player->angle, NULL, SDL_FLIP_NONE, 127);
			blitAndUpdateSpriteAnimatedEX(player->shipFlame, player->x - player->shipSprite->w * 0.5 - player->shipFlame->w, player->y - player->shipFlame->h * 0.5,
				SC_TOP_LEFT, player->angle, &(SDL_Point){(player->shipSprite->w) * 0.5 + player->shipFlame->w, player->shipFlame->h * 0.5}, SDL_FLIP_NONE, 127);
		}
	}

	//only happens if app.debug = true
	displayCollider(app.renderer, &COLOR_RED, player->collider);
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
	player->hp = 3;	//3 hits before dying
	player->iFrames = 0;
	player->weaponType = WT_NORMAL;
	player->shipSprite = initSprite(app.gameplaySprites, 0, 11, 3, 3);
	player->shipFlame = initSpriteAnimated(app.gameplaySprites, 16, 15, 1, 1, 4, 0, 0.25, AL_LOOP);
	player->collider = initOBBCollider(player->shipSprite->w * 0.2, player->shipSprite->h * 0.2, (Vector2){ player->x, player->y }, player->angle);
}

//destruct player
void deletePlayer() {
	free(player->shipSprite);
	free(player->shipFlame);
	free(player->collider);
	free(player);
}