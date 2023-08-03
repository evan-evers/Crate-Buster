#ifndef GEOMETRY_H
#define GEOMETRY_H

/*
* Constants, structs and functions related to geometry.
*/

#define RADIANS_TO_DEGREES (double)180/M_PI
#define DEGREES_TO_RADIANS (double)M_PI/180
//constants - x axis (I), y axis (J), 0 vector
#define VECTOR2_I (Vector2){ 1, 0 }	//note: this and VECTOR2_J don't seem to work right; their x and y values both equal 0 when you refer to them
#define VECTOR2_J (Vector2){ 0, 1 }
#define VECTOR2_0 (Vector2){ 0, 0 }

typedef struct {
	float x;
	float y;
} Vector2;

Vector2 addVec2(Vector2 a, Vector2 b);
Vector2 multVec2(Vector2 a, Vector2 b);
Vector2 scalarMultVec2(Vector2 a, float b);
struct SDL_Point vec2ToSDL_Point(const Vector2 vec);
void rotateVector(Vector2 *vec, float *angle, Vector2 origin);
void normalize(Vector2* vec);
float dotProduct(const Vector2* a, const Vector2* b);
float distanceSquared(float x1, float y1, float x2, float y2);
Vector2 projectVector(const Vector2* vProj, const Vector2* vOnto);

#endif