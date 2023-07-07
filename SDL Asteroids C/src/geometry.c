#include "common.h"

#include "geometry.h"

//normalizes a vector
void normalize(Vector2* vector) {
	float magnitude = sqrt(vector->x*vector->x + vector->y*vector->y);
	if (magnitude != 0) {
		vector->x /= magnitude;
		vector->y /= magnitude;
	}
}