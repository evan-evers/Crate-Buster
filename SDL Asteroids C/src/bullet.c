#include "common.h"
#include "float.h"

#include "bullets.h"

#include "crates.h"
#include "player.h"
#include "stage.h"

extern App app;
extern InputManager input;
extern Player* player;
extern Stage stage;

void firePlayerBullet(void);
static void fireNormalBullet();
static void fireErraticBullet();
static void fireBouncerBullet();
static void fireShotgunBullet();
void updateBullets(void);
void drawBullets(void);
static void deleteBullet(Bullet *b);
void deleteBullets(void);

static float bulletOffset = 20;	//offset from the center of the player when a bullet is created
static float erraticAccel = 0.08;	//per-frame acceleration for erratic bullets
static float shotgunDeccel = 0.08;	//per-frame decceleration for erratic bullets
static float shotgunDespawnThreshold = 1.0f;	//speed threshold at which shotgun bullets despawn

static const int BULLET_NORMAL_DMG = 15;
static const int BULLET_ERRATIC_DMG = 8;
static const int BULLET_BOUNCER_DMG = 20;
static const int BULLET_SHOTGUN_DMG = 10;

//handles the player firing a bullet
void firePlayerBullet(void) {
	switch (player->weaponType) {
		case(WT_NORMAL):
			fireNormalBullet();
			break;

		case(WT_ERRATIC):
			fireErraticBullet();
			break;

		case(WT_BOUNCER):
			fireBouncerBullet();
			break;

		case(WT_SHOTGUN):
			fireShotgunBullet();
			break;
	}
}

static void fireNormalBullet() {
	Bullet *b;

	//allocate and add to list
	b = malloc(sizeof(Bullet));
	memset(b, 0, sizeof(Bullet));
	if (stage.bulletHead == NULL) {
		stage.bulletHead = b;
		stage.bulletTail = b;
	} else {
		stage.bulletTail->next = b;
		stage.bulletTail = b;
	}

	//initialize
	b->dirVector.x = cos(player->angle * DEGREES_TO_RADIANS);
	b->dirVector.y = sin(player->angle * DEGREES_TO_RADIANS);
	b->x = player->x + b->dirVector.x * bulletOffset;	//direction vector * bullet offset is added in order to offset the bullet from the center of the player sprite
	b->y = player->y + b->dirVector.y * bulletOffset;
	b->speed = 6;
	b->angle = atan2(b->dirVector.y, b->dirVector.x) * RADIANS_TO_DEGREES;
	b->ttl = FPS * 3;	//3 seconds to live
	b->type = WT_NORMAL;
	b->sprite = initSpriteAnimated(app.gameplaySprites, 16, 0, 2, 1, 4, 0, 0.25f, AL_LOOP);
	b->collider = initOBBCollider(b->sprite->w * 0.5, b->sprite->h * 0.5, (Vector2){b->x, b->y}, b->angle * DEGREES_TO_RADIANS);
	b->next = NULL;	//always inserting on end of list

	player->reload = 10;
}

//a bullet type with high fire rate, random spread, random speed, and velocity that increases slightly with every update
static void fireErraticBullet() {
	Bullet *b;

	//allocate and add to list
	b = malloc(sizeof(Bullet));
	memset(b, 0, sizeof(Bullet));
	if (stage.bulletHead == NULL) {
		stage.bulletHead = b;
		stage.bulletTail = b;
	} else {
		stage.bulletTail->next = b;
		stage.bulletTail = b;
	}

	//initialize
	//randomize direction slightly
	b->dirVector.x = cos((player->angle + randFloatRange(-30, 30)) * DEGREES_TO_RADIANS);
	b->dirVector.y = sin((player->angle + randFloatRange(-30, 30)) * DEGREES_TO_RADIANS);
	b->x = player->x + b->dirVector.x * bulletOffset;	//direction vector * bullet offset is added in order to offset the bullet from the center of the player sprite
	b->y = player->y + b->dirVector.y * bulletOffset;
	b->speed = 4 + randFloatRange(-2,2);	//slightly random speed
	b->angle = atan2(b->dirVector.y, b->dirVector.x) * RADIANS_TO_DEGREES;
	b->ttl = FPS * 2;	//2 seconds to live
	b->type = WT_ERRATIC;
	b->sprite = initSpriteAnimated(app.gameplaySprites, 16, 3, 1, 1, 4, 0, 0.5f, AL_LOOP);
	b->collider = initOBBCollider(b->sprite->w * 0.5, b->sprite->h * 0.5, (Vector2) { b->x, b->y }, b->angle * DEGREES_TO_RADIANS);
	b->next = NULL;	//always inserting on end of list

	player->reload = 4;
}

static void fireBouncerBullet() {
	Bullet *b;

	//allocate and add to list
	b = malloc(sizeof(Bullet));
	memset(b, 0, sizeof(Bullet));
	if (stage.bulletHead == NULL) {
		stage.bulletHead = b;
		stage.bulletTail = b;
	} else {
		stage.bulletTail->next = b;
		stage.bulletTail = b;
	}

	//initialize
	b->dirVector.x = cos(player->angle * DEGREES_TO_RADIANS);
	b->dirVector.y = sin(player->angle * DEGREES_TO_RADIANS);
	b->x = player->x + b->dirVector.x * bulletOffset;	//direction vector * bullet offset is added in order to offset the bullet from the center of the player sprite
	b->y = player->y + b->dirVector.y * bulletOffset;
	b->speed = 4;
	b->angle = atan2(b->dirVector.y, b->dirVector.x) * RADIANS_TO_DEGREES;
	b->ttl = FPS * 4;	//4 seconds to live
	b->type = WT_BOUNCER;
	b->sprite = initSpriteAnimated(app.gameplaySprites, 16, 6, 2, 1, 4, 0, 0.25f, AL_LOOP);
	b->collider = initOBBCollider(b->sprite->w * 0.5, b->sprite->h * 0.5, (Vector2) { b->x, b->y }, b->angle * DEGREES_TO_RADIANS);
	b->next = NULL;	//always inserting on end of list

	player->reload = 30;
}

static void fireShotgunBullet() {
	Bullet *b;

	int numBullets = randIntRange(8,11);	//8 - 10 bullets shot per shot
	for (int i = 0; i < numBullets; ++i) {
		//allocate and add to list
		b = malloc(sizeof(Bullet));
		memset(b, 0, sizeof(Bullet));
		if (stage.bulletHead == NULL) {
			stage.bulletHead = b;
			stage.bulletTail = b;
		}
		else {
			stage.bulletTail->next = b;
			stage.bulletTail = b;
		}

		//initialize
		b->dirVector.x = cos((player->angle + randFloatRange(-30, 30)) * DEGREES_TO_RADIANS);
		b->dirVector.y = sin((player->angle + randFloatRange(-30, 30)) * DEGREES_TO_RADIANS);
		b->x = player->x + b->dirVector.x * bulletOffset;	//direction vector * bullet offset is added in order to offset the bullet from the center of the player sprite
		b->y = player->y + b->dirVector.y * bulletOffset;
		b->speed = 7 + randFloatRange(-1, 1);	//slightly random speed
		b->angle = atan2(b->dirVector.y, b->dirVector.x) * RADIANS_TO_DEGREES;
		b->ttl = FPS * 5;	//5 seconds to live
		b->type = WT_SHOTGUN;
		b->sprite = initSpriteAnimated(app.gameplaySprites, 16, 9, 1, 1, 4, 0, 0.25f, AL_LOOP);
		b->collider = initOBBCollider(b->sprite->w * 0.5, b->sprite->h * 0.5, (Vector2) { b->x, b->y }, b->angle * DEGREES_TO_RADIANS);
		b->next = NULL;	//always inserting on end of list
	}

	player->reload = 40;
}

//moves and deletes bullets
void updateBullets(void) {
	Bullet *b, *prev;

	b = prev = stage.bulletHead;

	//how far off of the screen to go before screenwrapping
	//0.75 is just an approximation of the square root of 2 (1.414 something) divided by 2
	//w is multiplied by this in case the sprite is rotated at a ~45 degree angle
	float margin = 0.75, horzEdgeDist, vertEdgeDist;

	while(b != NULL) {
		//update position of element
		b->x += b->dirVector.x * b->speed;
		b->y += b->dirVector.y * b->speed;

		//take care of some bullet-type dependent things
		switch (b->type) {
			case(WT_ERRATIC):
				b->speed += erraticAccel;
				break;
			case(WT_SHOTGUN):
				b->speed -= shotgunDeccel;

				//despawn if speed is too low
				if (b->speed <= shotgunDespawnThreshold)
					b->ttl = 0;
				break;
		}

		//screenwrap
		horzEdgeDist = b->sprite->w * margin;
		vertEdgeDist = b->sprite->h * margin;
		if (b->x < -horzEdgeDist)
			b->x = SCREEN_WIDTH + horzEdgeDist;
		if (b->x > SCREEN_WIDTH + horzEdgeDist)
			b->x = -horzEdgeDist;
		if (b->y < -vertEdgeDist)
			b->y = SCREEN_HEIGHT + vertEdgeDist;
		if (b->y > SCREEN_HEIGHT + vertEdgeDist)
			b->y = -vertEdgeDist;

		//update collider
		updateCollider(b->collider, b->x, b->y, FLT_MAX, -1, -1);

		//check for a collision with a crate
		Crate *crate = stage.crateHead;

		while (crate != NULL) {
			if (checkCollision(b->collider, crate->collider)) {
				//if there's a collision, apply damage based on bullet type
				switch (b->type) {
				case(WT_NORMAL):
					crate->hp -= BULLET_NORMAL_DMG;
					break;
				case(WT_ERRATIC):
					crate->hp -= BULLET_ERRATIC_DMG;
					break;
				case(WT_BOUNCER):
					crate->hp -= BULLET_BOUNCER_DMG;
					
					//make bullet bounce
					Vector2 *normalVec;

					break;
				case(WT_SHOTGUN):
					crate->hp -= BULLET_SHOTGUN_DMG;
					break;
				}

				//destroy bullet
				b->ttl = 0;
			}

			crate = crate->next;
		}

		//update ttl and destroy bullet if it's hit the end of ttl
		if (--b->ttl <= 0) {
			//edge case: last element
			if (b == stage.bulletTail) {
				//update bulletTail so it's not pointing to a freed element
				stage.bulletTail = prev;
			}
			
			if (b == stage.bulletHead) {
				//edge case: deleting first element
				stage.bulletHead = b->next;
				deleteBullet(b);
				b = prev = stage.bulletHead;	//update b and prev
				//no incrementation here
			} else {
				//general case
				prev->next = b->next;
				deleteBullet(b);
				b = prev->next;	//move b to next element
			}
		} else {
			//increment (must be in else, as deletion operations naturally increment)
			prev = b;
			b = b->next;
		}
	}
}

void drawBullets(void) {
	Bullet *b;

	for (b = stage.bulletHead; b != NULL; b = b->next) {
		blitAndUpdateSpriteAnimatedEX(b->sprite, b->x, b->y, SC_CENTER, b->angle, NULL, SDL_FLIP_NONE, 255);

		displayCollider(app.renderer, &COLOR_RED, b->collider);
	}
}

//no initialization function needed

//for deleting one bullet. frees dynamically allocated texture, then bullet itself.
static void deleteBullet(Bullet *b) {
	free(b->sprite);
	free(b);
	b = NULL;	//not really necessary but good practice
}

//for deleting all bullets.
void deleteBullets(void) {
	//clean linked list
	Bullet *b;

	b = stage.bulletHead;

	while (b != NULL) {
		//delete
		stage.bulletHead = b->next;
		deleteBullet(b);
		b = stage.bulletHead;
	}

	b = stage.bulletHead = stage.bulletTail = NULL;
}