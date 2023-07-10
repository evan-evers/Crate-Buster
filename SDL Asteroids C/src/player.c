#include "common.h"

#include "player.h"
#include "geometry.h"

extern App app;
extern InputManager input;

static const float PLAYER_SPEED_MAX = 3;
static const int DASH_TIME_MAX = 20;	//maximum amount of time spent in dash state
static const float DASH_SPEED_DECAY = 0.30;	//speed decay per frame in dash state
static const float DASH_SPEED_MAX = 10;	//maximum speed at start of dash state
static const float ACCEL = 0.25;	//acceleration
static const float DECCEL = 0.125;	//decelleration

static void psNormal(Player* player) {
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

	//state changes
	if (input.dash) {
		//if player isn't inputting a direction, dash towards the cursor
		if (input.leftLR == 0 && input.leftUD == 0) {
			player->dirVector.x = cos(player->angle * DEGREES_TO_RADIANS);
			player->dirVector.y = sin(player->angle * DEGREES_TO_RADIANS);
		}

		player->speed = DASH_SPEED_MAX;
		player->dashTimer = DASH_TIME_MAX;
		player->state = PS_DASHING;
	}
}

static void psDashing(Player* player) {
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

	//state changes

	//decrement dash timer
	if (--player->dashTimer == 0) {
		player->state = PS_NORMAL;
	}
}

//update player every game step
void updatePlayer(Player* player) {
	switch (player->state) {
		case(PS_NORMAL):
			psNormal(player);
		break;
		case(PS_DASHING):
			psDashing(player);
		break;
		default:
			psNormal(player);
	}
}

//draw player
void drawPlayer(Player* player) {
	//draw sprite at center of player
	blitSpriteEX(player->shipSprite, player->x - player->shipSprite->w * 0.5, player->y - player->shipSprite->h * 0.5, SC_TOP_LEFT, player->angle, NULL, SDL_FLIP_NONE, 255);
	//flame's just magic-numbered into place
	//flame's rotation origin is the center of the ship
	blitAndUpdateSpriteAnimatedEX(player->shipFlame, player->x - player->shipSprite->w * 0.5 - player->shipFlame->w, player->y - player->shipFlame->h * 0.5,
		SC_TOP_LEFT, player->angle, &(SDL_Point){(player->shipSprite->w) * 0.5 + player->shipFlame->w, player->shipFlame->h * 0.5}, SDL_FLIP_NONE, 255);
}



//initialize player at pos (x,y)
Player* initPlayer(int x, int y) {
	Player* player = malloc(sizeof(Player));
	player->x = x;
	player->y = y;
	player->angle = 0;
	player->speed = 0;		//total speed
	player->state = PS_NORMAL;
	player->shipSprite = initSprite(app.gameplaySprites, 0, 11, 3, 3);
	player->shipFlame = initSpriteAnimated(app.gameplaySprites, 16, 15, 1, 1, 4, 0, 0.25, AL_LOOP);

	return player;
}

//destruct player
void deletePlayer(Player* player) {
	free(player->shipSprite);
	free(player->shipFlame);
	free(player);
}