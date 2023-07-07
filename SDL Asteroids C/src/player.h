#ifndef PLAYER_H
#define PLAYER_H

/*
* Player struct and functions.
* Might want to center the x and y of the player, as right now they're top left.
*/

#include "draw.h"

typedef struct {
	float x;
	float y;
	float angle;	//in degrees for now
	Sprite* shipSprite;
	SpriteAnimated* shipFlame;
} Player;

void updatePlayer(Player* player);
void drawPlayer(Player* player);

//initializer and destructor
Player* initPlayer(int x, int y);
void deletePlayer(Player* player);

#endif