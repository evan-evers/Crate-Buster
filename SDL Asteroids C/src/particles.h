#ifndef PARTICLES_H
#define PARTICLES_H

#include "draw.h"

/*
* A generalized particle engine.
*/

//Generic particle
typedef struct Particle Particle;
struct Particle {
	float x;
	float y;
	float deltaX;	//x distance moved per frame
	float deltaY;	//y distance moved per frame
	float angle;	//angle to be drawn at
	int ttl;	//time to live
	SpriteAnimated *sprite;
	void (*update)(Particle *particle);
	void (*draw)(Particle *particle);
	Particle *next;
};

////struct to hold a particle
//typedef struct ParticleArray ParticleArray;
//struct ParticleArray{
//	Particle *array;
//	ParticleArray *next;
//};

Particle *initParticle(SpriteAnimated *sprite, float x, float y, float deltaX, float deltaY, float angle, int ttl, void (*update)(void), void (*draw)(void));
void updateParticles(void);
void drawParticles(void);
void deleteParticles(void);
void explosionDraw(Particle *particle);

#endif