#ifndef BULLETS_H
#define BULLETS_H

/*
* Enums, structs and functions that deal with player and enemy bullets
*/

#include "colliders.h"
#include "enemies.h"
#include "geometry.h"

//describes the types of bullets there are in the game
typedef enum {
	BT_NORMAL,
	BT_ERRATIC,
	BT_BOUNCER,
	BT_SHOTGUN,
	BT_ENEMY
} BulletType;

//Catch-all bullet struct
typedef struct Bullet Bullet;
struct Bullet {
	float x;		//x position
	float y;		//y position
	float speed;	//total speed
	float angle;	//in degrees for now
	Vector2 dirVector;	//stores bullet direction (should always be normalized)
	int ttl;		//time to live
	BulletType type;		//the type of weapon this bullet was fired from (aka the type of bullet this is)
	SpriteAnimated *sprite;
	OBBCollider *collider;
	Bullet *next;
};

void firePlayerBullet(void);
void fireEnemyBullet(Enemy *enemy);
void updateBullets(void);
void drawBullets(void);
void deleteBullets(void);

#endif