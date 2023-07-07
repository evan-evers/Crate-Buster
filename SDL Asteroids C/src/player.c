#include "common.h"

#include "player.h"
#include "geometry.h"

extern App app;
extern InputManager input;

static float playerSpeed = 2.5;
static Vector2 playerDirVector;	//stores player direction (should always be normalized)

//update player every game step
void updatePlayer(Player* player) {
	//movement

	//calculate movement vector
	playerDirVector.x = input.lr;
	playerDirVector.y = input.ud;
	normalize(&playerDirVector);

	//update player movement
	player->x += playerDirVector.x * playerSpeed;
	player->y += playerDirVector.y * playerSpeed;

	//clamp player to edges of screen
	if (player->x < 0)
		player->x = 0;
	if (player->x > SCREEN_WIDTH)
		player->x = SCREEN_WIDTH;
	if (player->y < 0)
		player->y = 0;
	if (player->y > SCREEN_HEIGHT)
		player->y = SCREEN_HEIGHT;

	//angle spaceship towards mouse
	player->angle = atan2((input.mouse.y - player->y), (input.mouse.x - player->x)) * (180/M_PI);
}

//draw player
void drawPlayer(Player* player) {
	//draw sprite at center of player
	blitSpriteEX(player->shipSprite, player->x - player->shipSprite->w * 0.5, player->y - player->shipSprite->h * 0.5, player->angle, NULL, SDL_FLIP_NONE, 255);
	//flame's just magic-numbered into place
	//flame's rotation origin is the center of the ship
	blitAndUpdateSpriteAnimatedEX(player->shipFlame, player->x - player->shipSprite->w * 0.5 - player->shipFlame->w, player->y - player->shipFlame->h * 0.5,
		player->angle, &(SDL_Point){(player->shipSprite->w) * 0.5 + player->shipFlame->w, player->shipFlame->h * 0.5}, SDL_FLIP_NONE, 255);
}



//initialize player at pos (x,y)
Player* initPlayer(int x, int y) {
	Player* player = malloc(sizeof(Player));
	player->x = x;
	player->y = y;
	player->angle = 0;
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