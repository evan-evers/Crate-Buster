#ifndef BULLETS_H
#define BULLETS_H

/*
* Enums, structs and functions that deal with player and enemy bullets
*/

#include "colliders.h"
#include "geometry.h"

//describes the types of weapons the player can fire
typedef enum {
	WT_NORMAL,
	WT_ERRATIC,
	WT_BOUNCER,
	WT_SHOTGUN,
	WT_ENEMY
} WeaponType;

//Catch-all bullet struct
typedef struct Bullet Bullet;
struct Bullet {
	float x;		//x position
	float y;		//y position
	float speed;	//total speed
	float angle;	//in degrees for now
	Vector2 dirVector;	//stores bullet direction (should always be normalized)
	int ttl;		//time to live
	WeaponType type;		//the type of weapon this bullet was fired from (aka the type of bullet this is)
	SpriteAnimated* sprite;
	OBBCollider* collider;
	Bullet* next;
};

void firePlayerBullet(void);
void updateBullets(void);
void drawBullets(void);
void deleteBullets(void);

#endif