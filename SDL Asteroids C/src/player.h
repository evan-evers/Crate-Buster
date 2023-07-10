#ifndef PLAYER_H
#define PLAYER_H

/*
* Player struct and functions.
* Might want to center the x and y of the player, as right now they're top left.
*/

#include "draw.h"
#include "geometry.h"

typedef enum {
	PS_NORMAL,
	PS_DASHING,
	PS_DESTROYED
} playerState;

typedef struct {
	float x;		//x position
	float y;		//y position
	float speed;	//total speed
	float angle;	//in degrees for now
	playerState state;	//store current state
	Vector2 dirVector;	//stores player direction (should always be normalized)
	int dashTimer;	//stores how much longer a dash will go on for
	Sprite* shipSprite;
	SpriteAnimated* shipFlame;
} Player;

void updatePlayer(Player* player);
void drawPlayer(Player* player);

//initializer and destructor
Player* initPlayer(int x, int y);
void deletePlayer(Player* player);

#endif