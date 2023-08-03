#ifndef BACKGROUND_H
#define BACKGROUND_H

/*
* Header file for the game's background.
* The background is drawn as a planet on top of three layers of stars.
* Layer 1 is the closest, layer 3 is the furthest.
*/

#include "particles.h"

typedef struct {
	Particle *planet;
	Particle *starsLayer1[NUM_BACKGROUND_STARS_L1];
	Particle *starsLayer2[NUM_BACKGROUND_STARS_L2];
	Particle *starsLayer3[NUM_BACKGROUND_STARS_L3];
	int backgroundFlashRedTimer;	//timer to make sure the background red flash fades out after the player is hit
	int backgroundFlashWhiteTimer;	//ditto for green flash
} Background;

void initBackground(void);
void drawBackground(void);
void deleteBackground(void);

#endif