#include "common.h"
#include "float.h"

#include "crates.h"

#include "colliders.h"
#include "geometry.h"
#include "stage.h"

extern App app;
extern Stage stage;

void updateCrates(void);
void drawCrates(void);
void initCrates(int numberOfCrates);
void addCrate(void);
static void deleteCrate(Crate* crate);
void deleteCrates(void);

static const int SPACE_FOR_PLAYER = 50;	//a margin around the edges of where the player spawns, so asteroids don't spawn on player

void updateCrates(void) {
	Crate *crate, *prev;
	crate = prev = stage.crateHead;

	//how far off of the screen to go before screenwrapping
	//0.75 is just an approximation of the square root of 2 (1.414 something) divided by 2
	//w is multiplied by this in case the sprite is rotated at a ~45 degree angle
	float margin = 0.75, horzEdgeDist, vertEdgeDist;

	while (crate != NULL) {
		//update angle, position, and collider
		crate->angle += crate->angleSpeed;
		crate->x += crate->dirVector.x * crate->speed;
		crate->y += crate->dirVector.y * crate->speed;
		updateCollider(crate->collider, crate->x, crate->y, crate->angle * DEGREES_TO_RADIANS, -1, -1);
		
		//screenwrap
		horzEdgeDist = crate->sprite->w * margin;
		vertEdgeDist = crate->sprite->h * margin;
		if (crate->x < -horzEdgeDist)
			crate->x = SCREEN_WIDTH + horzEdgeDist;
		if (crate->x > SCREEN_WIDTH + horzEdgeDist)
			crate->x = -horzEdgeDist;
		if (crate->y < -vertEdgeDist)
			crate->y = SCREEN_HEIGHT + vertEdgeDist;
		if (crate->y > SCREEN_HEIGHT + vertEdgeDist)
			crate->y = -vertEdgeDist;

		//destroy a crate if its hp is 0
		//notably, this function doesn't ever subtract from a crate's hp; that only happens in update functions for objects that interact with crates
		if (crate->hp <= 0) {
			//edge case: last element
			if (crate == stage.crateTail) {
				//update bulletTail so it's not pointing to a freed element
				stage.crateTail = prev;
			}

			if (crate == stage.crateHead) {
				//edge case: deleting first element
				stage.crateHead = crate->next;
				deleteCrate(crate);
				crate = prev = stage.crateHead;	//update b and prev
				//no incrementation here
			}
			else {
				//general case
				prev->next = crate->next;
				deleteCrate(crate);
				crate = prev->next;	//move b to next element
			}
		}
		else {
			//increment (must be in else, as deletion operations naturally increment)
			prev = crate;
			crate = crate->next;
		}
	}
}

void drawCrates(void) {
	Crate* c = stage.crateHead;

	while (c != NULL) {
		blitSpriteEX(c->sprite, c->x, c->y, SC_CENTER, c->angle, NULL, SDL_FLIP_NONE, 255);

		//only happens if app.debug = true
		displayCollider(app.renderer, &COLOR_RED, c->collider);

		c = c->next;
	}
}

void initCrates(int numberOfCrates) {
	for(int i = 0; i < numberOfCrates; ++i)
		addCrate();
}

void addCrate(void) {
	//allocate and add to list
	Crate* c = malloc(sizeof(Crate));
	memset(c, 0, sizeof(Crate));	//set all values to 0 as a precaution (it's really easy to forget to initialize stuff)
	if (stage.crateHead == NULL) {
		stage.crateHead = c;
		stage.crateTail = c;
	}
	else {
		stage.crateTail->next = c;
		stage.crateTail = c;
	}

	//initialize
	//for now, all crates are initialized as big ones
	c->sprite = initSpriteAnimated(app.gameplaySprites, 0, 0, 4, 4, 2, 0, 0, AL_LOOP);
	//spawn at random spot in screen (but not on top of the player, who spawns in the middle of the screen)
	if (rand() % 2 == 0)
		c->x = randFloatRange(0,SCREEN_WIDTH * 0.5 - SPACE_FOR_PLAYER);
	else
		c->x = randFloatRange(SCREEN_WIDTH * 0.5 + SPACE_FOR_PLAYER, SCREEN_WIDTH);
	if (rand() % 2 == 0)
		c->y = randFloatRange(0, SCREEN_HEIGHT * 0.5 - SPACE_FOR_PLAYER);
	else
		c->y = randFloatRange(SCREEN_HEIGHT * 0.5 + SPACE_FOR_PLAYER, SCREEN_HEIGHT);

	c->angle = randFloat(359.999999999999);	//start at random angle
	c->angleSpeed = randFloatRange(-3, 3);

	c->speed = randFloatRange(1,2);
	float dirAngle = randFloat(359.999999999999);
	c->dirVector.x = cos(dirAngle);
	c->dirVector.y = sin(dirAngle);
	c->collider = initOBBCollider(c->sprite->w * 0.45, c->sprite->h * 0.45, (Vector2){c->x, c->y}, c->angle * DEGREES_TO_RADIANS);

	c->hp = 100;
}

static void deleteCrate(Crate *crate) {
	free(crate->sprite);
	free(crate->collider);
	free(crate);
	crate == NULL;
}

void deleteCrates(void) {
	//clean linked list
	Crate* crate = stage.crateHead;

	while (crate != NULL) {
		stage.crateHead = crate->next;
		deleteCrate(crate);
		crate = stage.crateHead;
	}

	crate = stage.crateHead = stage.crateTail = NULL;
}