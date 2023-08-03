#ifndef CRATES_H
#define CRATES_H

#include "colliders.h"
#include "draw.h"
#include "geometry.h"

/*
* Stuff related to this game's equivalent of asteroids.
*/

typedef enum {
	CT_LARGE,
	CT_MEDIUM,
	CT_SMALL
} CrateType;

typedef struct Crate Crate;
struct Crate {
	float x;		//x position
	float y;		//y position
	float speed;	//total speed
	float angle;	//in degrees for now
	float angleSpeed;	//rotation speed; also in degrees
	int hp;			//hit points
	int timeSinceDamaged;	//record time since last damaged in order to make sprite flashing on hit work
	CrateType type;
	Vector2 dirVector;	//stores direction (should always be normalized)
	SpriteStatic *crateSprite;
	SpriteStatic *crateSpriteHitflash;
	OBBCollider* collider;
	Crate* next;	//linked list
};

void updateCrates(void);
void drawCrates(void);
void initCrates(int numberOfCrates);
void addCrate(CrateType type, int x, int y);
void deleteCrates(void);

#endif