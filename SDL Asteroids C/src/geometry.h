#ifndef GEOMETRY_H
#define GEOMETRY_H

/*
* Structs and functions related to geometry.
*/

typedef struct {
	float x;
	float y;
} Vector2;

void normalize(Vector2* vector);

#endif