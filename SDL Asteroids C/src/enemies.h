#ifndef ENEMIES_H
#define ENEMIES_H

#include "colliders.h"
#include "draw.h"
#include "particles.h"
#include "geometry.h"

/*
* Header files for enemies.
*/

//enemy states
typedef enum {
	ES_ENTER_STAGE,
	ES_NORMAL
} EnemyState;

//enemy struct
typedef struct Enemy Enemy;
struct Enemy {
	float x;		//x position
	float y;		//y position
	float speed;	//total speed
	float angle;	//angle at which the object faces; in degrees for now
	EnemyState state;	//stores current state
	Vector2 dirVector;	//stores direction (should always be normalized)
	int reload;		//when it's below zero, the enemy can fire again
	int hp;			//hit points
	int timeSinceDamaged;	//hitflash management variable
	SpriteStatic *sprite;
	SpriteStatic *spriteHitflash;
	SpriteAnimated *spriteFlame;
	Particle *muzzleFlash;
	OBBCollider *collider;
	Enemy *next;
};

void initEnemy(void);
void updateEnemies(void);
void spawnEnemies(void);
void drawEnemies(void);
void deleteEnemy(Enemy *enemy);
void deleteEnemies(void);

#endif