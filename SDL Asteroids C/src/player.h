#ifndef PLAYER_H
#define PLAYER_H

/*
* Player struct and functions.
* Might want to center the x and y of the player, as right now they're top left.
*/

#include "bullets.h"
#include "colliders.h"
#include "draw.h"
#include "geometry.h"
#include "particles.h"

typedef enum {
	PS_NORMAL,
	PS_DASHING,
	PS_DESTROYED
} PlayerState;

typedef struct {
	float x;		//x position
	float y;		//y position
	float speed;	//total speed
	float angle;	//in degrees for now
	PlayerState state;	//store current state
	Vector2 dirVector;	//stores player's intended direction (should always be normalized)
	Vector2 momentumVector;	//stores player's actual direction and speed (should always be normalized)
	int dashTimer;	//stores how much longer a dash will go on for
	int reload;		//when it's below zero, the player can fire again
	int hp;			//hit points
	int iFrames;	//invincibility frames
	BulletType weaponType;	//what type of weapon the player has right now
	SpriteStatic *shipSprite;
	SpriteAnimated *shipFlame;
	Particle *muzzleFlash;
	OBBCollider *collider;
} Player;

void updatePlayer();
void drawPlayer();

//initializer and destructor
//also the reset function
void initPlayer(int x, int y);
void resetPlayer(int x, int y);
void deletePlayer();

#endif