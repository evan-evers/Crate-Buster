#ifndef STAGE_H
#define STAGE_H

#include "bullets.h"
#include "crates.h"
#include "enemies.h"
#include "particles.h"

/*
* Various elements related to the current stage
*/

//game states
typedef enum {
	SS_BEGINNING,
	SS_GAMEPLAY,
	SS_END,
	SS_GAME_OVER,
	SS_INPUT_HIGHSCORE,	//player goes here on a game over if their score is high enough to get on the table
	SS_HIGHSCORE_TABLE,		//player goes here if their score isn't high enough
	SS_PAUSED
} StageState;

//holds key variables for managing a stage
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
	//current level the player's on
	int level;
	//current game state
	StageState state;
} Stage;

//stage functions
void initStage(void);
void deleteStage(void);

//options menu widget actions
//these could probably use their own file
void waFullscreenToggle(void);
void waSFXSlider(void);
void waMusicSlider(void);

#endif