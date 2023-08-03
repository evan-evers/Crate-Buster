#ifndef STAGE_H
#define STAGE_H

#include "bullets.h"
#include "crates.h"
#include "enemies.h"
#include "particles.h"

/*
* Various elements related to the current stage
*/

typedef struct {
	//linked lists of the objects in the stage
	Bullet *bulletHead, *bulletTail;
	Crate *crateHead, *crateTail;
	Enemy *enemyHead, *enemyTail;
	Particle *particleHead, *particleTail;	//NOTE: not every particle should be in this list.

	//timer keeping track of how long the player's been on this level for
	int timer;
	//player score (in amount of scrap collected)
	int score;
} Stage;

void initStage(void);
void deleteStage(void);

#endif