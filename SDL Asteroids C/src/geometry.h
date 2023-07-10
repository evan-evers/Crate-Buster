#ifndef GEOMETRY_H
#define GEOMETRY_H

/*
* Constants, structs and functions related to geometry.
*/

#define RADIANS_TO_DEGREES (double)180/M_PI
#define DEGREES_TO_RADIANS (double)M_PI/180

typedef struct {
	float x;
	float y;
} Vector2;

void normalize(Vector2* vector);

#endif