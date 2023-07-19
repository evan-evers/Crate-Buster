#ifndef STAGE_H
#define STAGE_H

#include "bullets.h"
#include "crates.h"

/*
* Various elements related to the current stage
*/

typedef struct {
	Bullet *bulletHead, *bulletTail;
	Crate* crateHead, * crateTail;
} Stage;

#endif