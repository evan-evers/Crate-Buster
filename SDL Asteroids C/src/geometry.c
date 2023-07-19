#include <stdlib.h>
#include <math.h>

#include "geometry.h"

#include "common.h"

Vector2 addVec2(Vector2 a, Vector2 b);
Vector2 multVec2(Vector2 a, Vector2 b);
Vector2 scalarMultVec2(Vector2 a, float b);
struct SDL_Point vec2ToSDL_Point(const Vector2 vect);
void normalize(Vector2* vector);
float dotProduct(const Vector2* a, const Vector2* b);
float distanceSquared(float x1, float y1, float x2, float y2);
Vector2* projectVector(const Vector2* vProj, const Vector2* vOnto);

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
struct SDL_Point vec2ToSDL_Point(const Vector2 vect) {
	SDL_Point pFinal = { 0, 0 };
	pFinal.x = (int)vect.x;
	pFinal.y = (int)vect.y;

	return pFinal;
}

//rotates a vector.
//don't know why on earth passing in an angle by value causes that angle to be 0, but whatever.
void rotateVector(Vector2* vect, float* angle, Vector2 origin) {
	//simply rotating the vector results in floating point errors that stack up very quickly
	//these errors drastically change the length of a vector in a matter of dozens of rotations
	//therefore, we store the vector's magnitude and give it back that magnitude by the end of the function

	//convert vector's position to be relative to the origin
	vect->x -= origin.x;
	vect->y -= origin.y;

	Vector2 vectTemp;
	vectTemp.x = vect->x;
	vectTemp.y = vect->y;

	//rotate about selected origin
	vectTemp.x = vect->x * cos(*angle) - vect->y * sin(*angle);
	vectTemp.y = vect->x * sin(*angle) + vect->y * cos(*angle);

	//convert back to global space
	vectTemp.x += origin.x;
	vectTemp.y += origin.y;

	//set passed-in vector equal to vectTemp
	vect->x = vectTemp.x;
	vect->y = vectTemp.y;
}

//normalizes a vector
void normalize(Vector2* vector) {
	float magnitude = sqrtf(vector->x*vector->x + vector->y*vector->y);
	if (magnitude != 0) {
		vector->x /= magnitude;
		vector->y /= magnitude;
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
Vector2* projectVector(const Vector2 *vProj, const Vector2 *vOnto) {
	Vector2 vFinal;
	float scalar = dotProduct(vProj,vOnto) * dotProduct(vOnto,vOnto);
	vFinal.x = scalar * vOnto->x;
	vFinal.y = scalar * vOnto->y;
	return &vFinal;
}