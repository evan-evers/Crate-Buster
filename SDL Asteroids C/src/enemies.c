#include "common.h"

#include "enemies.h"

#include "bullets.h"
#include "draw.h"
#include "geometry.h"
#include "particles.h"
#include "player.h"
#include "powerups.h"
#include "scrap.h"
#include "stage.h"

extern App app;
extern Player *player;
extern Stage stage;

void initEnemy(void);
void updateEnemies(void);
void spawnEnemies(void);
void drawEnemies(void);
void deleteEnemy(Enemy *enemy);
void deleteEnemies(void);

static const int ENEMY_HEALTH_MAX = 125;
static const int SPAWN_DISTANCE = 200;	//distance away from the edges of the screen where an enemy can spawn

static int enemySpawnTimer = FPS * 15;	//wait 15 seconds before spawning any enemies

//muzzle flash update is handled in update function

//muzzle flash particle draw behavior
static void muzzleFlashDraw(Particle *particle) {
	//only draw if muzzle flash isn't at the end of its animation
	if (particle->sprite->currentFrame != particle->sprite->frames - 1) {
		blitAndUpdateSpriteAnimated(particle->sprite, particle->x, particle->y);
	}
}

//adds an enemy to the stage
void initEnemy(void) {
	Enemy *enemy;
	
	enemy = malloc(sizeof(Enemy));
	memset(enemy, 0, sizeof(Enemy));
	if (stage.enemyHead == NULL) {
		stage.enemyHead = enemy;
		stage.enemyTail = enemy;
	}
	else {
		stage.enemyTail->next = enemy;
		stage.enemyTail = enemy;
	}
	
	//spawn the enemy somewhere on the edge of a rectangle around the stage
	int random = rand() % 4;
	switch (random) {
	case(0):
		enemy->x = -SPAWN_DISTANCE;
		enemy->y = randFloatRange(-SPAWN_DISTANCE, SCREEN_HEIGHT + SPAWN_DISTANCE);
		break;
	case(1):
		enemy->x = SCREEN_WIDTH + SPAWN_DISTANCE;
		enemy->y = randFloatRange(-SPAWN_DISTANCE, SCREEN_HEIGHT + SPAWN_DISTANCE);
		break;
	case(2):
		enemy->x = randFloatRange(-SPAWN_DISTANCE, SCREEN_WIDTH + SPAWN_DISTANCE);
		enemy->y = -SPAWN_DISTANCE;
		break;
	case(3):
		enemy->x = randFloatRange(-SPAWN_DISTANCE, SCREEN_WIDTH + SPAWN_DISTANCE);
		enemy->y = SCREEN_HEIGHT + SPAWN_DISTANCE;
		break;
	}

	enemy->speed = randFloatRange(1, 2);
	enemy->angle = atan2((player->y - enemy->y), (player->x - enemy->x)) * RADIANS_TO_DEGREES;
	enemy->state = ES_ENTER_STAGE;
	enemy->dirVector.x = cos(enemy->angle * DEGREES_TO_RADIANS);	//enemy starts out moving towards the player
	enemy->dirVector.y = sin(enemy->angle * DEGREES_TO_RADIANS);
	enemy->reload = FPS * 3;	//enemy shouldn't fire immediately
	enemy->hp = ENEMY_HEALTH_MAX;
	enemy->sprite = initSpriteStatic(app.gameplaySprites, 6, 7, 4, 4, SC_CENTER);
	enemy->spriteHitflash = initSpriteStatic(app.gameplaySprites, 10, 7, 4, 4, SC_CENTER);
	enemy->spriteFlame = initSpriteAnimated(app.gameplaySprites, 16, 16, 1, 1, SC_CENTER, 4, 0, 0.25, AL_LOOP);
	enemy->muzzleFlash = initParticle(initSpriteAnimated(app.gameplaySprites, 16, 14, 2, 2, SC_CENTER, 5, 5, 0.2, AL_ONESHOT), enemy->x, enemy->y, 0, 0, 0, 1, NULL, muzzleFlashDraw);
	enemy->collider = initOBBCollider(enemy->sprite->w * 0.35, enemy->sprite->h * 0.35, (Vector2){enemy->x, enemy->y}, enemy->angle);
	enemy->next = NULL;
}

static void esEnterStage(Enemy *enemy) {
	//update position (that's literally all that needs to be done while in this state
	enemy->x += enemy->dirVector.x * enemy->speed;
	enemy->y += enemy->dirVector.y * enemy->speed;

	//state change
	
	//check if enemy is inbounds; if they are, switch to normal state
	float horzEdgeDist = enemy->sprite->w * SCREENWRAP_MARGIN;
	float vertEdgeDist = enemy->sprite->h * SCREENWRAP_MARGIN;
	if (enemy->x >= -horzEdgeDist && enemy->x <= SCREEN_WIDTH + horzEdgeDist && enemy->y >= -vertEdgeDist && enemy->y <= SCREEN_HEIGHT + vertEdgeDist)
		enemy->state = ES_NORMAL;
}

static void esNormal(Enemy *enemy) {
	//update enemy's angle
	enemy->angle = atan2((player->y - enemy->y), (player->x - enemy->x)) * RADIANS_TO_DEGREES;

	//update position
	enemy->x += enemy->dirVector.x * enemy->speed;
	enemy->y += enemy->dirVector.y * enemy->speed;

	//screenwrap
	float horzEdgeDist = enemy->sprite->w * SCREENWRAP_MARGIN;
	float vertEdgeDist = enemy->sprite->h * SCREENWRAP_MARGIN;
	if (enemy->x < -horzEdgeDist)
		enemy->x = SCREEN_WIDTH + horzEdgeDist;
	if (enemy->x > SCREEN_WIDTH + horzEdgeDist)
		enemy->x = -horzEdgeDist;
	if (enemy->y < -vertEdgeDist)
		enemy->y = SCREEN_HEIGHT + vertEdgeDist;
	if (enemy->y > SCREEN_HEIGHT + vertEdgeDist)
		enemy->y = -vertEdgeDist;

	//update collider
	updateCollider(enemy->collider, enemy->x, enemy->y, enemy->angle * DEGREES_TO_RADIANS, -1, -1);

	//firing guns (updates reload)
	if (--enemy->reload <= 0) {
		fireEnemyBullet(enemy);
		
		//muzzle flash
		enemy->muzzleFlash->sprite->currentFrame = 0;
	}
}

//updates enemy behavior
void updateEnemies(void) {
	Enemy *enemy, *prev;
	enemy = prev = stage.enemyHead;

	while (enemy != NULL) {
		//go through each enemy's state machine
		switch (enemy->state) {
		case(ES_ENTER_STAGE):
			esEnterStage(enemy);
			break;
		case(ES_NORMAL):
			esNormal(enemy);
			break;
		}

		//handle muzzle flash if its animation isn't finished for now
		if (enemy->muzzleFlash->sprite->currentFrame != enemy->muzzleFlash->sprite->frames - 1) {
			//put muzzle flash in correct position
			enemy->muzzleFlash->x = enemy->x + cos(enemy->angle * DEGREES_TO_RADIANS) * BULLET_OFFSET_ENEMY + 4;
			enemy->muzzleFlash->y = enemy->y + sin(enemy->angle * DEGREES_TO_RADIANS) * BULLET_OFFSET_ENEMY + 4;
		}

		//update hitflash timer
		++enemy->timeSinceDamaged;

		//if an enemy's lost its HP, delete it
		if (enemy->hp <= 0) {
			//death explosion
			initParticle(initSpriteAnimated(app.gameplaySprites, 0, 18, 4, 4, SC_CENTER, 5, 0, 0.3, AL_ONESHOT), enemy->x, enemy->y, 0, 0, (float)(rand() % 4) * 90, 1, NULL, explosionDraw);

			//add 10 scrap pieces to stage
			for (int i = 0; i < 10; ++i)
				initScrap(enemy->x + randFloatRange(-20, 20), enemy->y + randFloatRange(-20, 20));

			//50% chance to add a powerup
			if (rand() % 2) {
				initPowerup(enemy->x, enemy->y);
			}

			//edge case: last element
			if (enemy == stage.enemyTail) {
				//update tail so it's not pointing to a freed element
				stage.enemyTail = prev;
			}

			if (enemy == stage.enemyHead) {
				//edge case: deleting first element
				stage.enemyHead = enemy->next;
				deleteEnemy(enemy);
				enemy = prev = stage.enemyHead;	//update ptrs
				//no incrementation here
			} else {
				//general case
				prev->next = enemy->next;
				deleteEnemy(enemy);
				enemy = prev->next;	//move ptr to next element
			}
		} else {
			//increment (must be in else, as deletion operations naturally increment)
			prev = enemy;
			enemy = enemy->next;
		}
	}
}

void spawnEnemies(void) {
	if (--enemySpawnTimer <= 0) {
		initEnemy();

		//the longer the player's been in the level, the faster enemies should spawn in (to an extent)
		enemySpawnTimer = MAX(FPS * 10 - (float)stage.timer * 0.05, FPS * 3);
	}
}

//draws enemies
void drawEnemies(void) {
	Enemy *enemy = stage.enemyHead;

	while (enemy != NULL) {
		//draw muzzle flash if there's something to be drawn
		if (enemy->muzzleFlash->sprite->currentFrame != enemy->muzzleFlash->sprite->frames - 1)
			blitAndUpdateSpriteAnimatedEX(enemy->muzzleFlash->sprite, enemy->muzzleFlash->x, enemy->muzzleFlash->y, enemy->angle, NULL, SDL_FLIP_NONE);

		//draw enemy
		if (enemy->timeSinceDamaged < END_OF_FLASH) {
			//draw hitflash over normal sprite, decreasing its transparency the longer it's been since the enemy was hit
			blitSpriteStaticEX(enemy->sprite, enemy->x, enemy->y, enemy->angle, NULL, SDL_FLIP_NONE);
			setTextureRGBA(enemy->sprite->atlas->texture, 255, 255, 255, (int)(255 * (float)(END_OF_FLASH - enemy->timeSinceDamaged) / (float)END_OF_FLASH));
			blitSpriteStaticEX(enemy->spriteHitflash, enemy->x, enemy->y, enemy->angle, NULL, SDL_FLIP_NONE);
			setTextureRGBA(enemy->sprite->atlas->texture, 255, 255, 255, 255);
		} else
			blitSpriteStaticEX(enemy->sprite, enemy->x, enemy->y, enemy->angle, NULL, SDL_FLIP_NONE);

		//draw flame
		blitAndUpdateSpriteAnimatedEX(enemy->spriteFlame, enemy->x - 28, enemy->y, enemy->angle, &(SDL_Point) {enemy->spriteFlame->w * 0.5 + 28, enemy->spriteFlame->h * 0.5}, SDL_FLIP_NONE);

		//only happens if app.debug = true
		displayCollider(app.renderer, &COLOR_RED, enemy->collider);

		enemy = enemy->next;
	}
}

//deletes an enemy
void deleteEnemy(Enemy *enemy) {
	free(enemy->sprite);
	free(enemy->spriteFlame);
	free(enemy->collider);
	enemy->muzzleFlash->ttl = 0;
	free(enemy);
	enemy = NULL;
}

//deletes all enemies in the stage
void deleteEnemies(void) {
	Enemy *enemy = stage.enemyHead;

	while (enemy != NULL) {
		stage.enemyHead = enemy->next;
		deleteEnemy(enemy);
		enemy = stage.enemyHead;
	}

	stage.enemyHead = stage.enemyTail = NULL;
}