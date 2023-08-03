#include <stdlib.h>
#include <math.h>

#include "geometry.h"

#include "common.h"

Vector2 addVec2(Vector2 a, Vector2 b);
Vector2 multVec2(Vector2 a, Vector2 b);
Vector2 scalarMultVec2(Vector2 a, float b);
struct SDL_Point vec2ToSDL_Point(const Vector2 vec);
void normalize(Vector2* vec);
float dotProduct(const Vector2* a, const Vector2* b);
float distanceSquared(float x1, float y1, float x2, float y2);
Vector2 projectVector(const Vector2* vProj, const Vector2* vOnto);

//adds vectors and returns a pointer to the sum
Vector2 addVec2(Vector2 a, Vector2 b) {
	Vector2 vFinal;
	vFinal.x = a.x + b.x;
	vFinal.y = a.y + b.y;

	return vFinal;
}

//multiplies vectors and returns a pointer to the sum
Vector2 multVec2(Vector2 a, Vector2 b) {
	Vector2 vFinal;
	vFinal.x = a.x * b.x;
	vFinal.y = a.y * b.y;

	return vFinal;
}

//multiplies vector by a scalar
Vector2 scalarMultVec2(Vector2 a, float b) {
	Vector2 vFinal;
	vFinal.x = a.x * b;
	vFinal.y = a.y * b;

	return vFinal;
}

//convert a vector to an SDL_Point
struct SDL_Point vec2ToSDL_Point(const Vector2 vec) {
	SDL_Point pFinal = { 0, 0 };
	pFinal.x = (int)vec.x;
	pFinal.y = (int)vec.y;

	return pFinal;
}

//rotates a vector.
//don't know why on earth passing in an angle by value causes that angle to be 0, but whatever.
void rotateVector(Vector2* vec, float* angle, Vector2 origin) {
	//simply rotating the vector results in floating point errors that stack up very quickly
	//these errors drastically change the length of a vector in a matter of dozens of rotations
	//therefore, we store the vector's magnitude and give it back that magnitude by the end of the function

	//convert vector's position to be relative to the origin
	vec->x -= origin.x;
	vec->y -= origin.y;

	Vector2 vectTemp;
	vectTemp.x = vec->x;
	vectTemp.y = vec->y;

	//rotate about selected origin
	vectTemp.x = vec->x * cos(*angle) - vec->y * sin(*angle);
	vectTemp.y = vec->x * sin(*angle) + vec->y * cos(*angle);

	//convert back to global space
	vectTemp.x += origin.x;
	vectTemp.y += origin.y;

	//set passed-in vector equal to vectTemp
	vec->x = vectTemp.x;
	vec->y = vectTemp.y;
}

//normalizes a vector
void normalize(Vector2* vec) {
	float magnitude = sqrtf(vec->x*vec->x + vec->y*vec->y);
	if (magnitude != 0) {
		vec->x /= magnitude;
		vec->y /= magnitude;
	}
}

//standard 2D dot product
float dotProduct(const Vector2 *a, const Vector2 *b) {
	return (float)(a->x * b->x + a->y * b->y);
}

//returns distance squared between two points; very useful for collision checks
float distanceSquared(float x1, float y1, float x2, float y2) {
	return (float)((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
}

//returns vProj projected onto vOnto
Vector2 projectVector(const Vector2 *vProj, const Vector2 *vOnto) {
	Vector2 vFinal;
	float scalar = dotProduct(vProj,vOnto) * dotProduct(vOnto,vOnto);
	vFinal = scalarMultVec2(*vOnto, scalar);
	return vFinal;
}