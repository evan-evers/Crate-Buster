#include "common.h"
#include "float.h"

#include "crates.h"

#include "colliders.h"
#include "geometry.h"
#include "particles.h"
#include "player.h"
#include "scrap.h"
#include "stage.h"

extern App app;
extern Player *player;
extern Stage stage;

void updateCrates(void);
void drawCrates(void);
void initCrates(int numberOfCrates);
void addCrate(CrateType type, int x, int y);
static void deleteCrate(Crate* crate);
void deleteCrates(void);

static const int SPACE_FOR_PLAYER = 50;	//a margin around the edges of where the player spawns, so asteroids don't spawn on player

void updateCrates(void) {
	Crate *crate, *prev;
	crate = prev = stage.crateHead;

	float horzEdgeDist, vertEdgeDist;

	while (crate != NULL) {
		//update angle, position, and collider
		crate->angle += crate->angleSpeed;
		crate->x += crate->dirVector.x * crate->speed;
		crate->y += crate->dirVector.y * crate->speed;
		updateCollider(crate->collider, crate->x, crate->y, crate->angle * DEGREES_TO_RADIANS, -1, -1);
		
		//screenwrap
		horzEdgeDist = crate->crateSprite->w * SCREENWRAP_MARGIN;
		vertEdgeDist = crate->crateSprite->h * SCREENWRAP_MARGIN;
		if (crate->x < -horzEdgeDist)
			crate->x = SCREEN_WIDTH + horzEdgeDist;
		if (crate->x > SCREEN_WIDTH + horzEdgeDist)
			crate->x = -horzEdgeDist;
		if (crate->y < -vertEdgeDist)
			crate->y = SCREEN_HEIGHT + vertEdgeDist;
		if (crate->y > SCREEN_HEIGHT + vertEdgeDist)
			crate->y = -vertEdgeDist;

		//update hitflash timer
		++crate->timeSinceDamaged;

		//destroy a crate if its hp is 0
		//notably, this function doesn't ever subtract from a crate's hp; that only happens in update functions for objects that interact with crates
		if (crate->hp <= 0) {
			//create new crates depending on type
			//also crate scrap depending on type
			int n;
			switch (crate->type) {
			case(CT_LARGE):
				addCrate(CT_MEDIUM, crate->x, crate->y);
				addCrate(CT_MEDIUM, crate->x, crate->y);
				//7-9 pieces of scrap
				n = randIntRange(7, 9);
				for (int i = 0; i < n; ++i)
					initScrap(crate->x + randFloatRange(-20, 20), crate->y + randFloatRange(-20, 20));
				break;
			case(CT_MEDIUM):
				addCrate(CT_SMALL, crate->x, crate->y);
				addCrate(CT_SMALL, crate->x, crate->y);
				//3-5 pieces of scrap
				n = randIntRange(3, 5);
				for (int i = 0; i < n; ++i)
					initScrap(crate->x + randFloatRange(-15, 15), crate->y + randFloatRange(-15, 15));
				break;
			case(CT_SMALL):
				//1-3 pieces of scrap
				n = randIntRange(1, 3);
				for (int i = 0; i < n; ++i)
					initScrap(crate->x + randFloatRange(-10, 10), crate->y + randFloatRange(-10, 10));
				break;
			}

			//explosion particle
			initParticle(initSpriteAnimated(app.gameplaySprites, 0, 18, 4, 4, SC_CENTER, 5, 0, 0.3, AL_ONESHOT), crate->x, crate->y, 0, 0, (float)(rand() % 4) * 90, 1, NULL, explosionDraw);

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
	Crate* crate = stage.crateHead;

	while (crate != NULL) {
		//draw crate
		if (crate->timeSinceDamaged < END_OF_FLASH) {
			//draw hitflash over normal sprite, decreasing its transparency the longer it's been since the crate was hit
			blitSpriteStaticEX(crate->crateSprite, crate->x, crate->y, crate->angle, NULL, SDL_FLIP_NONE);
			setTextureRGBA(crate->crateSprite->atlas->texture, 255, 255, 255, (int)(255 * (float)(END_OF_FLASH - crate->timeSinceDamaged) / (float)END_OF_FLASH));
			blitSpriteStaticEX(crate->crateSpriteHitflash, crate->x, crate->y, crate->angle, NULL, SDL_FLIP_NONE);
			setTextureRGBA(crate->crateSprite->atlas->texture, 255, 255, 255, 255);
		}
		else
			blitSpriteStaticEX(crate->crateSprite, crate->x, crate->y, crate->angle, NULL, SDL_FLIP_NONE);

		//only happens if app.debug = true
		displayCollider(app.renderer, &COLOR_RED, crate->collider);

		crate = crate->next;
	}
}

//addCrates would be a better name for this function
void initCrates(int numberOfCrates) {
	int x = 0, y = 0;
	for (int i = 0; i < numberOfCrates; ++i) {
		//spawn at random spot in screen (but not on top of the player, who spawns in the middle of the screen)
		if (rand() % 2 == 0)
			x = randFloatRange(0, SCREEN_WIDTH * 0.5 - SPACE_FOR_PLAYER);
		else
			x = randFloatRange(SCREEN_WIDTH * 0.5 + SPACE_FOR_PLAYER, SCREEN_WIDTH);
		if (rand() % 2 == 0)
			y = randFloatRange(0, SCREEN_HEIGHT * 0.5 - SPACE_FOR_PLAYER);
		else
			y = randFloatRange(SCREEN_HEIGHT * 0.5 + SPACE_FOR_PLAYER, SCREEN_HEIGHT);

		addCrate(CT_LARGE, x, y);
	}
}

void addCrate(CrateType type, int x, int y) {
	//allocate and add to list
	Crate* crate = malloc(sizeof(Crate));
	memset(crate, 0, sizeof(Crate));	//set all values to 0 as a precaution (it's really easy to forget to initialize stuff)
	if (stage.crateHead == NULL) {
		stage.crateHead = crate;
		stage.crateTail = crate;
	}
	else {
		stage.crateTail->next = crate;
		stage.crateTail = crate;
	}

	//initialize
	crate->type = type;
	switch (crate->type) {
	case(CT_LARGE):
		crate->crateSprite = initSpriteStatic(app.gameplaySprites, 0, 0, 4, 4, SC_CENTER);
		crate->crateSpriteHitflash = initSpriteStatic(app.gameplaySprites, 4, 0, 4, 4, SC_CENTER);
		crate->hp = 100;
		crate->speed = randFloatRange(1.0 + (stage.level - 1) * 0.2, 1.5 + (stage.level - 1) * 0.2);
		break;
	case(CT_MEDIUM):
		crate->crateSprite = initSpriteStatic(app.gameplaySprites, 0, 4, 3, 3, SC_CENTER);
		crate->crateSpriteHitflash = initSpriteStatic(app.gameplaySprites, 3, 4, 3, 3, SC_CENTER);
		crate->hp = 50;
		crate->speed = randFloatRange(1.5 + (stage.level - 1) * 0.2, 2.0 + (stage.level - 1) * 0.2);
		break;
	case(CT_SMALL):
		crate->crateSprite = initSpriteStatic(app.gameplaySprites, 0, 7, 2, 2, SC_CENTER);
		crate->crateSpriteHitflash = initSpriteStatic(app.gameplaySprites, 2, 7, 2, 2, SC_CENTER);
		crate->hp = 25;
		crate->speed = randFloatRange(2.0 + (stage.level - 1) * 0.2, 2.5 + (stage.level - 1) * 0.2);
		break;
	}

	crate->x = x;
	crate->y = y;
	crate->angle = randFloat(359.999999999999);	//start at random angle
	crate->angleSpeed = randFloatRange(-3, 3);
	crate->timeSinceDamaged = END_OF_FLASH;	//don't want crates to give hitflash when spawned
	crate->collider = initOBBCollider(crate->crateSprite->w * 0.45, crate->crateSprite->h * 0.45, (Vector2) { crate->x, crate->y }, crate->angle *DEGREES_TO_RADIANS);

	//spawn crate moving in a random direction that is perpendicular or away from the player
	Vector2 vectToPlayer;
	float dirAngle;
	do {
		dirAngle = randFloat(359.999999999999);
		crate->dirVector.x = cos(dirAngle);
		crate->dirVector.y = sin(dirAngle);
		vectToPlayer.x = crate->x - player->x;
		vectToPlayer.y = crate->y - player->y;
	} while(dotProduct(&vectToPlayer, &crate->dirVector) < 0);
}

static void deleteCrate(Crate *crate) {
	free(crate->crateSprite);
	free(crate->crateSpriteHitflash);
	free(crate->collider);
	free(crate);
	crate = NULL;
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