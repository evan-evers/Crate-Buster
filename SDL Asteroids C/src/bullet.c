#include "common.h"

#include "bullets.h"
#include "player.h"
#include "stage.h"

extern App app;
extern InputManager input;
extern Player* player;
extern Stage stage;

//bullet textures
static SpriteAnimated* sprNormalBullet;
static SpriteAnimated* sprShotgunBullet;
static SpriteAnimated* sprErraticBullet;
static SpriteAnimated* sprBouncerBullet;
static SpriteAnimated* sprEnemyBullet;

static void fireNormalBullet();
static void fireErraticBullet();
static void fireBouncerBullet();
static void fireShotgunBullet();

static float BULLET_OFFSET = 20;	//offset from the center of the player when a bullet is created
static float ERRATIC_ACCEL = 0.08;	//per-frame acceleration for erratic bullets
static float SHOTGUN_DECCEL = 0.08;	//per-frame decceleration for erratic bullets
static float SHOTGUN_DESPAWN_THRESHOLD = 1.0f;	//speed threshold at which shotgun bullets despawn

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
	b->x = player->x + b->dirVector.x * BULLET_OFFSET;	//direction vector * bullet offset is added in order to offset the bullet from the center of the player sprite
	b->y = player->y + b->dirVector.y * BULLET_OFFSET;
	b->speed = 5;
	b->angle = atan2(b->dirVector.y, b->dirVector.x) * RADIANS_TO_DEGREES;
	b->ttl = FPS * 5;	//5 seconds to live
	b->type = WT_NORMAL;
	b->bulletSprite = sprNormalBullet;
	b->next = NULL;	//always inserting on end of list

	player->reload = 8;
}

//a bullet type with high fire rate, random spread, random speed, and velocity that increases slightly with every update
static void fireErraticBullet() {
	Bullet *b;

	//allocate and add to list
	b = malloc(sizeof(Bullet));
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
	b->x = player->x + b->dirVector.x * BULLET_OFFSET;	//direction vector * bullet offset is added in order to offset the bullet from the center of the player sprite
	b->y = player->y + b->dirVector.y * BULLET_OFFSET;
	b->speed = 4 + randFloatRange(-2,2);	//slightly random speed
	b->angle = atan2(b->dirVector.y, b->dirVector.x) * RADIANS_TO_DEGREES;
	b->ttl = FPS * 5;	//5 seconds to live
	b->type = WT_ERRATIC;
	b->bulletSprite = sprErraticBullet;
	b->next = NULL;	//always inserting on end of list

	player->reload = 4;
}

static void fireBouncerBullet() {
	Bullet *b;

	//allocate and add to list
	b = malloc(sizeof(Bullet));
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
	b->x = player->x + b->dirVector.x * BULLET_OFFSET;	//direction vector * bullet offset is added in order to offset the bullet from the center of the player sprite
	b->y = player->y + b->dirVector.y * BULLET_OFFSET;
	b->speed = 4;
	b->angle = atan2(b->dirVector.y, b->dirVector.x) * RADIANS_TO_DEGREES;
	b->ttl = FPS * 5;	//5 seconds to live
	b->type = WT_BOUNCER;
	b->bulletSprite = sprBouncerBullet;
	b->next = NULL;	//always inserting on end of list

	player->reload = 20;
}

static void fireShotgunBullet() {
	Bullet *b;

	int numBullets = randIntRange(8,10);	//8 - 10 bullets shot per shot
	for (int i = 0; i < numBullets; ++i) {
		//allocate and add to list
		b = malloc(sizeof(Bullet));
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
		b->x = player->x + b->dirVector.x * BULLET_OFFSET;	//direction vector * bullet offset is added in order to offset the bullet from the center of the player sprite
		b->y = player->y + b->dirVector.y * BULLET_OFFSET;
		b->speed = 6 + randFloatRange(-1, 1);	//slightly random speed
		b->angle = atan2(b->dirVector.y, b->dirVector.x) * RADIANS_TO_DEGREES;
		b->ttl = FPS * 5;	//5 seconds to live
		b->type = WT_SHOTGUN;
		b->bulletSprite = sprShotgunBullet;
		b->next = NULL;	//always inserting on end of list
	}

	player->reload = 40;
}

//moves and deletes bullets
void updateBullets(void) {
	Bullet *b, *prev;

	b = prev = stage.bulletHead;

	while(b != NULL) {
		//update position of element
		b->x += b->dirVector.x * b->speed;
		b->y += b->dirVector.y * b->speed;

		//take care of some bullet-type dependent things
		switch (b->type) {
			case(WT_ERRATIC):
				b->speed += ERRATIC_ACCEL;
				break;
			case(WT_SHOTGUN):
				b->speed -= SHOTGUN_DECCEL;

				//despawn if speed is too low
				if (b->speed <= SHOTGUN_DESPAWN_THRESHOLD)
					b->ttl = 0;
				break;
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
				free(b);
				b = prev = stage.bulletHead;	//update b and prev
				//no incrementation here
			} else {
				//general case
				prev->next = b->next;
				free(b);
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
		blitAndUpdateSpriteAnimatedEX(b->bulletSprite, b->x, b->y, SC_CENTER, b->angle, NULL, SDL_FLIP_NONE, 255);
	}
}

void initBullets(void) {
	sprNormalBullet = initSpriteAnimated(app.gameplaySprites, 16, 0, 2, 1, 4, 0, 0.03f, AL_LOOP);
	sprErraticBullet = initSpriteAnimated(app.gameplaySprites, 16, 3, 1, 1, 4, 0, 0.03f, AL_LOOP);
	sprBouncerBullet = initSpriteAnimated(app.gameplaySprites, 16, 6, 2, 1, 4, 0, 0.03f, AL_LOOP);
	sprShotgunBullet = initSpriteAnimated(app.gameplaySprites, 16, 9, 1, 1, 4, 0, 0.03f, AL_LOOP);
	sprEnemyBullet = initSpriteAnimated(app.gameplaySprites, 16, 12, 2, 1, 4, 0, 0.03f, AL_LOOP);
}

void deleteBullets(void) {
	//clean linked list
	Bullet *b;

	b = stage.bulletHead;

	while (b != NULL) {
		//delete
		stage.bulletHead = b->next;
		free(b);
		b = stage.bulletHead;
	}

	b = stage.bulletHead = stage.bulletTail = NULL;

	//free sprites
	free(sprNormalBullet);
	free(sprErraticBullet);
	free(sprBouncerBullet);
	free(sprShotgunBullet);
	free(sprEnemyBullet);
}