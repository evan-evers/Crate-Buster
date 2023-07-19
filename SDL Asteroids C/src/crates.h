#ifndef CRATES_H
#define CRATES_H

#include "colliders.h"
#include "geometry.h"

/*
* Stuff related to this game's equivalent of asteroids.
*/

typedef struct Crate Crate;
struct Crate {
	float x;		//x position
	float y;		//y position
	float speed;	//total speed
	float angle;	//in degrees for now
	float angleSpeed;	//also in degrees
	int hp;	//hit points
	Vector2 dirVector;	//stores direction (should always be normalized)
	SpriteAnimated* sprite;
	OBBCollider* collider;
	Crate* next;	//linked list
};

void updateCrates(void);
void drawCrates(void);
void initCrates(int numberOfCrates);
void addCrate(void);
void deleteCrates(void);

#endif