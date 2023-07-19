#ifndef COLLIDERS_H
#define COLLIDERS_H

#include "geometry.h"

/*
* Structs and functions related to collisions
* Don't forget: these colliders are malloced, so make sure to free them
*/

//Oriented bounding box collider
typedef struct {
	//two axes which store orientation
	Vector2 axes[2];		//local x and y axes (x = axes[0], y = axes[1])
	//width and height of the bounding box
	float halfwidths[2];	//Positive halfwidths along x and y axes (x = halfwidths[0], y = halfwidths[1])
	//center of the collider
	Vector2 origin;
} OBBCollider;

OBBCollider* initOBBCollider(const float xHalfwidth, const float yHalfwidth, const Vector2 origin, const float angle);
void updateCollider(OBBCollider* collider, const float x, const float y, const float angle, const float w, const float h);
void displayCollider(SDL_Renderer* renderer, const SDL_Color* color, const OBBCollider* collider);
bool checkCollision(const OBBCollider *a, const OBBCollider *b);

#endif
