#include "common.h"
#include "float.h"

#include "bullets.h"

#include "background.h"
#include "crates.h"
#include "enemies.h"
#include "particles.h"
#include "player.h"
#include "sound.h"
#include "stage.h"

extern App app;
extern InputManager input;
extern Player* player;
extern Stage stage;
extern Background background;

void firePlayerBullet(void);
static void fireNormalBullet();
static void fireErraticBullet();
static void fireBouncerBullet();
static void fireShotgunBullet();
void fireEnemyBullet(Enemy *enemy);
static void biUpdate(Particle *particle);
static void biDraw(Particle *particle);
void updateBullets(void);
void drawBullets(void);
static void deleteBullet(Bullet *b);
void deleteBullets(void);

//gameplay vars
static float erraticAccel = 0.08;	//per-frame acceleration for erratic bullets
static float shotgunDeccel = 0.12;	//per-frame decceleration for erratic bullets
static float shotgunDespawnThreshold = 1.0f;	//speed threshold at which shotgun bullets despawn

//constants
static const int BULLET_NORMAL_DMG = 20;
static const int BULLET_ERRATIC_DMG = 8;
static const int BULLET_BOUNCER_DMG = 12;
static const int BULLET_SHOTGUN_DMG = 15;
static const int BULLET_ENEMY_DMG = 25;	//this is just the amount of damage the enemy bullet does to a crate, not to the player

//initialize and fire a bullet from the player
void firePlayerBullet(void) {
	switch (player->weaponType) {
		case(BT_NORMAL):
			fireNormalBullet();
			break;

		case(BT_ERRATIC):
			fireErraticBullet();
			break;

		case(BT_BOUNCER):
			fireBouncerBullet();
			break;

		case(BT_SHOTGUN):
			fireShotgunBullet();
			break;
	}
}

static void fireNormalBullet() {
	Bullet *bullet;

	//allocate and add to list
	bullet = calloc(1, sizeof(Bullet));
	if (stage.bulletHead == NULL) {
		stage.bulletHead = bullet;
		stage.bulletTail = bullet;
	} else {
		stage.bulletTail->next = bullet;
		stage.bulletTail = bullet;
	}

	//initialize
	bullet->dirVector.x = cos(player->angle * DEGREES_TO_RADIANS);
	bullet->dirVector.y = sin(player->angle * DEGREES_TO_RADIANS);
	bullet->x = player->x + bullet->dirVector.x * BULLET_OFFSET_PLAYER;	//direction vector * bullet offset is added in order to offset the bullet from the center of the player sprite
	bullet->y = player->y + bullet->dirVector.y * BULLET_OFFSET_PLAYER;
	bullet->speed = 6;
	bullet->angle = atan2(bullet->dirVector.y, bullet->dirVector.x) * RADIANS_TO_DEGREES;
	bullet->ttl = FPS * 3;	//3 seconds to live
	bullet->type = BT_NORMAL;
	bullet->sprite = initSpriteAnimated(app.gameplaySprites, 16, 0, 2, 1, SC_CENTER, 4, 0, 0.25f, AL_LOOP);
	bullet->collider = initOBBCollider(bullet->sprite->w * 0.5, bullet->sprite->h * 0.5, (Vector2){bullet->x, bullet->y}, bullet->angle * DEGREES_TO_RADIANS);
	bullet->next = NULL;	//always inserting on end of list

	player->reload = 10;
}

//a bullet type with high fire rate, random spread, random speed, and velocity that increases slightly with every update
static void fireErraticBullet() {
	Bullet *bullet;

	//allocate and add to list
	bullet = calloc(1, sizeof(Bullet));
	if (stage.bulletHead == NULL) {
		stage.bulletHead = bullet;
		stage.bulletTail = bullet;
	} else {
		stage.bulletTail->next = bullet;
		stage.bulletTail = bullet;
	}

	//initialize
	//randomize direction slightly
	bullet->dirVector.x = cos((player->angle + randFloatRange(-30, 30)) * DEGREES_TO_RADIANS);
	bullet->dirVector.y = sin((player->angle + randFloatRange(-30, 30)) * DEGREES_TO_RADIANS);
	bullet->x = player->x + bullet->dirVector.x * BULLET_OFFSET_PLAYER;	//direction vector * bullet offset is added in order to offset the bullet from the center of the player sprite
	bullet->y = player->y + bullet->dirVector.y * BULLET_OFFSET_PLAYER;
	bullet->speed = 4 + randFloatRange(-2,2);	//slightly random speed
	bullet->angle = randFloat(359.9999999999999999);
	bullet->ttl = FPS * 2;	//2 seconds to live
	bullet->type = BT_ERRATIC;
	bullet->sprite = initSpriteAnimated(app.gameplaySprites, 16, 3, 1, 1, SC_CENTER, 4, 0, 0.5f, AL_LOOP);
	bullet->collider = initOBBCollider(bullet->sprite->w * 0.5, bullet->sprite->h * 0.5, (Vector2) { bullet->x, bullet->y }, bullet->angle * DEGREES_TO_RADIANS);
	bullet->next = NULL;	//always inserting on end of list

	player->reload = 4;
}

static void fireBouncerBullet() {
	Bullet *bullet;

	//allocate and add to list
	bullet = calloc(1, sizeof(Bullet));
	if (stage.bulletHead == NULL) {
		stage.bulletHead = bullet;
		stage.bulletTail = bullet;
	} else {
		stage.bulletTail->next = bullet;
		stage.bulletTail = bullet;
	}

	//initialize
	bullet->dirVector.x = cos(player->angle * DEGREES_TO_RADIANS);
	bullet->dirVector.y = sin(player->angle * DEGREES_TO_RADIANS);
	bullet->x = player->x + bullet->dirVector.x * BULLET_OFFSET_PLAYER;	//direction vector * bullet offset is added in order to offset the bullet from the center of the player sprite
	bullet->y = player->y + bullet->dirVector.y * BULLET_OFFSET_PLAYER;
	bullet->speed = 8;
	bullet->angle = atan2(bullet->dirVector.y, bullet->dirVector.x) * RADIANS_TO_DEGREES;
	bullet->ttl = FPS * 4;	//4 seconds to live
	bullet->type = BT_BOUNCER;
	bullet->sprite = initSpriteAnimated(app.gameplaySprites, 16, 6, 2, 1, SC_CENTER, 4, 0, 0.25f, AL_LOOP);
	bullet->collider = initOBBCollider(bullet->sprite->w * 0.5, bullet->sprite->h * 0.5, (Vector2) { bullet->x, bullet->y }, bullet->angle * DEGREES_TO_RADIANS);
	bullet->next = NULL;	//always inserting on end of list

	player->reload = 30;
}

static void fireShotgunBullet() {
	Bullet *bullet;

	int numBullets = randIntRange(8,11);	//8 - 10 bullets shot per shot
	for (int i = 0; i < numBullets; ++i) {
		//allocate and add to list
		bullet = calloc(1, sizeof(Bullet));
		if (stage.bulletHead == NULL) {
			stage.bulletHead = bullet;
			stage.bulletTail = bullet;
		}
		else {
			stage.bulletTail->next = bullet;
			stage.bulletTail = bullet;
		}

		//initialize
		bullet->dirVector.x = cos((player->angle + randFloatRange(-30, 30)) * DEGREES_TO_RADIANS);
		bullet->dirVector.y = sin((player->angle + randFloatRange(-30, 30)) * DEGREES_TO_RADIANS);
		bullet->x = player->x + bullet->dirVector.x * BULLET_OFFSET_PLAYER;	//direction vector * bullet offset is added in order to offset the bullet from the center of the player sprite
		bullet->y = player->y + bullet->dirVector.y * BULLET_OFFSET_PLAYER;
		bullet->speed = 10 + randFloatRange(-1, 1);	//slightly random speed
		bullet->angle = atan2(bullet->dirVector.y, bullet->dirVector.x) * RADIANS_TO_DEGREES;
		bullet->ttl = FPS * 5;	//5 seconds to live
		bullet->type = BT_SHOTGUN;
		bullet->sprite = initSpriteAnimated(app.gameplaySprites, 16, 9, 1, 1, SC_CENTER, 4, 0, 0.25f, AL_LOOP);
		bullet->collider = initOBBCollider(bullet->sprite->w * 0.5, bullet->sprite->h * 0.5, (Vector2) { bullet->x, bullet->y }, bullet->angle * DEGREES_TO_RADIANS);
		bullet->next = NULL;	//always inserting on end of list
	}

	player->reload = 40;
}

//initialize and fire an enemy bullet
void fireEnemyBullet(Enemy *enemy) {
	Bullet *bullet;

	//allocate and add to list
	bullet = calloc(1, sizeof(Bullet));
	if (stage.bulletHead == NULL) {
		stage.bulletHead = bullet;
		stage.bulletTail = bullet;
	} else {
		stage.bulletTail->next = bullet;
		stage.bulletTail = bullet;
	}

	//initialize
	bullet->dirVector.x = cos(enemy->angle * DEGREES_TO_RADIANS);
	bullet->dirVector.y = sin(enemy->angle * DEGREES_TO_RADIANS);
	bullet->x = enemy->x + bullet->dirVector.x * BULLET_OFFSET_ENEMY;	//direction vector * bullet offset is added in order to offset the bullet from the center of the player sprite
	bullet->y = enemy->y + bullet->dirVector.y * BULLET_OFFSET_ENEMY;
	bullet->speed = 3;
	bullet->angle = atan2(bullet->dirVector.y, bullet->dirVector.x) * RADIANS_TO_DEGREES;
	bullet->ttl = FPS * 4;	//4 seconds to live
	bullet->type = BT_ENEMY;
	bullet->sprite = initSpriteAnimated(app.gameplaySprites, 16, 12, 2, 1, SC_CENTER, 4, 0, 0.25f, AL_LOOP);
	bullet->collider = initOBBCollider(bullet->sprite->w * 0.4, bullet->sprite->h * 0.4, (Vector2) { bullet->x, bullet->y }, bullet->angle *DEGREES_TO_RADIANS);	//slightly undersized collider for enemy bullets to bias things for the player
	bullet->next = NULL;	//always inserting on end of list

	enemy->reload = FPS * 3;
}

//bullet impact particle update function
static void biUpdate(Particle *particle) {
	//move according to delta
	particle->x += particle->deltaX;
	particle->y += particle->deltaY;
}

//bullet impact particle draw function
static void biDraw(Particle *particle) {
	//if on the extra frame, set the particle to be deleted
	if (particle->sprite->currentFrame == particle->sprite->frames - 1)
		particle->ttl = 0;

	blitAndUpdateSpriteAnimatedEX(particle->sprite, particle->x, particle->y, particle->angle, NULL, SDL_FLIP_NONE);
}

//moves and deletes bullets
void updateBullets(void) {
	Bullet *bullet, *prev;

	bullet = prev = stage.bulletHead;

	//vars for handling screenwrap
	float horzEdgeDist, vertEdgeDist;

	while(bullet != NULL) {
		//update position of element
		bullet->x += bullet->dirVector.x * bullet->speed;
		bullet->y += bullet->dirVector.y * bullet->speed;

		//take care of some bullet-type dependent things
		switch (bullet->type) {
			case(BT_ERRATIC):
				bullet->speed += erraticAccel;
				break;
			case(BT_SHOTGUN):
				bullet->speed -= shotgunDeccel;

				//despawn if speed is too low
				if (bullet->speed <= shotgunDespawnThreshold)
					bullet->ttl = 0;
				break;
		}

		//screenwrap
		horzEdgeDist = bullet->sprite->w * SCREENWRAP_MARGIN;
		vertEdgeDist = bullet->sprite->h * SCREENWRAP_MARGIN;
		if (bullet->x < -horzEdgeDist)
			bullet->x = SCREEN_WIDTH + horzEdgeDist;
		if (bullet->x > SCREEN_WIDTH + horzEdgeDist)
			bullet->x = -horzEdgeDist;
		if (bullet->y < -vertEdgeDist)
			bullet->y = SCREEN_HEIGHT + vertEdgeDist;
		if (bullet->y > SCREEN_HEIGHT + vertEdgeDist)
			bullet->y = -vertEdgeDist;

		//update collider
		updateCollider(bullet->collider, bullet->x, bullet->y, bullet->angle * DEGREES_TO_RADIANS, -1, -1);

		if (bullet->type != BT_ENEMY) {
			//handling a player bullet
			//check for a collision with a crate
			Crate *crate = stage.crateHead;

			while (crate != NULL) {
				if (checkIntersection(bullet->collider, crate->collider)) {
					//if there's a collision, apply damage based on bullet type
					switch (bullet->type) {
					case(BT_NORMAL):
						crate->hp -= BULLET_NORMAL_DMG;
						crate->timeSinceDamaged = 0;	//update this var for hitflash to work

						//destroy bullet
						bullet->ttl = 0;
						break;
					case(BT_ERRATIC):
						crate->hp -= BULLET_ERRATIC_DMG;
						crate->timeSinceDamaged = 0;	//update this var for hitflash to work

						//destroy bullet
						bullet->ttl = 0;
						break;
					case(BT_BOUNCER):
					{
						crate->hp -= BULLET_BOUNCER_DMG;
						crate->timeSinceDamaged = 0;	//update this var for hitflash to work

						//impact particle on bounce
						initParticle(initSpriteAnimated(app.gameplaySprites, 16, 7, 1, 1, SC_CENTER, 4, 0, 0.25, AL_ONESHOT), bullet->x, bullet->y, 0, 0, bullet->angle - 45, 1, NULL, biDraw);
						
						//make bullet bounce
						//to do this, we use the seperating axes thm to check which crate axis the center of the bullet is further away from
						//whichever axis the center is further from is the normal
						//we then reflect the bullet's direction vector across that normal
						//this is only approximately realistic; 
						//for instance, hitting a corner just defaults to the axes, and very high bullet speeds will result in inaccuracies
						Vector2 relativeOrigin, projAxis0, projAxis1, normal, velocity, projectedVelocity;
						float valAxis0, valAxis1;

						//find normal vector

						//need to use origin relative to the crate's origin, not origin relative to world space
						relativeOrigin.x = crate->collider->origin.x - bullet->collider->origin.x;
						relativeOrigin.y = crate->collider->origin.y - bullet->collider->origin.y;
						projAxis0 = projectVector(&relativeOrigin, &crate->collider->axes[0]);
						valAxis0 = fabs(dotProduct(&(crate->collider->axes[0]), &projAxis0));
						projAxis1 = projectVector(&relativeOrigin, &crate->collider->axes[1]);
						valAxis1 = fabs(dotProduct(&(crate->collider->axes[1]), &projAxis1));

						if (valAxis0 > valAxis1) {
							normal = projAxis0;
						} else {
							normal = projAxis1;
						}

						velocity = scalarMultVec2(bullet->dirVector, bullet->speed);			//bullet's velocity vector
						projectedVelocity = projectVector(&velocity, &normal);		//projection of velocity vector onto normal
						projectedVelocity = scalarMultVec2(projectedVelocity, 2);	//multiplied by 2 for projection trick

						bullet->dirVector.x = velocity.x - projectedVelocity.x;
						bullet->dirVector.y = velocity.y - projectedVelocity.y;
						normalize(&bullet->dirVector);	//dirVector should always be normalized

						//change angle to reflect change in direction
						bullet->angle = atan2(bullet->dirVector.y, bullet->dirVector.x) * RADIANS_TO_DEGREES;

						//cycle it through another change in velocity to prevent bouncing again on the next update (it's still probably within the crate on this update)
						bullet->x += bullet->dirVector.x * bullet->speed;
						bullet->y += bullet->dirVector.y * bullet->speed;

						break;
					}
					case(BT_SHOTGUN):
						crate->hp -= BULLET_SHOTGUN_DMG;
						crate->timeSinceDamaged = 0;	//update this var for hitflash to work

						//destroy bullet
						bullet->ttl = 0;
						break;

						//do nothing for enemy bullets
					}

					playSoundIsolated(SFX_ENEMY_HIT, SC_HITSOUNDS, false, bullet->x / SCREEN_WIDTH * 255.0);
				}

				crate = crate->next;
			}

			//check for a collision with an enemy
			Enemy *enemy = stage.enemyHead;

			while (enemy != NULL) {
				//only apply damage if enemy is in the stage proper
				if (checkIntersection(bullet->collider, enemy->collider) && enemy->state != ES_ENTER_STAGE) {
					//if there's a collision, apply damage based on bullet type
					switch (bullet->type) {
					case(BT_NORMAL):
						enemy->hp -= BULLET_NORMAL_DMG;
						enemy->timeSinceDamaged = 0;	//update this var for hitflash to work

						//destroy bullet
						bullet->ttl = 0;
						break;
					case(BT_ERRATIC):
						enemy->hp -= BULLET_ERRATIC_DMG;
						enemy->timeSinceDamaged = 0;	//update this var for hitflash to work

						//destroy bullet
						bullet->ttl = 0;
						break;
					case(BT_BOUNCER):
					{
						enemy->hp -= BULLET_BOUNCER_DMG;
						enemy->timeSinceDamaged = 0;	//update this var for hitflash to work

						//add impact particle on bounce
						initParticle(initSpriteAnimated(app.gameplaySprites, 16, 7, 1, 1, SC_CENTER, 4, 0, 0.25, AL_ONESHOT), bullet->x, bullet->y, 0, 0, bullet->angle - 45, 1, NULL, biDraw);

						//make bullet bounce
						//to do this, we use the seperating axes thm to check which enemy axis the center of the bullet is further away from
						//whichever axis the center is further from is the normal
						//we then reflect the bullet's direction vector across that normal
						//this is only approximately realistic; 
						//for instance, hitting a corner just defaults to the axes, and very high bullet speeds will result in inaccuracies
						Vector2 relativeOrigin, projAxis0, projAxis1, normal, velocity, projectedVelocity;
						float valAxis0, valAxis1;

						//find normal vector

						//need to use origin relative to the enemy's origin, not origin relative to world space
						relativeOrigin.x = enemy->collider->origin.x - bullet->collider->origin.x;
						relativeOrigin.y = enemy->collider->origin.y - bullet->collider->origin.y;
						projAxis0 = projectVector(&relativeOrigin, &enemy->collider->axes[0]);
						valAxis0 = fabs(dotProduct(&(enemy->collider->axes[0]), &projAxis0));
						projAxis1 = projectVector(&relativeOrigin, &enemy->collider->axes[1]);
						valAxis1 = fabs(dotProduct(&(enemy->collider->axes[1]), &projAxis1));

						if (valAxis0 > valAxis1) {
							normal = projAxis0;
						} else {
							normal = projAxis1;
						}

						velocity = scalarMultVec2(bullet->dirVector, bullet->speed);			//bullet's velocity vector
						projectedVelocity = projectVector(&velocity, &normal);		//projection of velocity vector onto normal
						projectedVelocity = scalarMultVec2(projectedVelocity, 2);	//multiplied by 2 for projection trick

						bullet->dirVector.x = velocity.x - projectedVelocity.x;
						bullet->dirVector.y = velocity.y - projectedVelocity.y;
						normalize(&bullet->dirVector);	//dirVector should always be normalized

						//change angle to reflect change in direction
						bullet->angle = atan2(bullet->dirVector.y, bullet->dirVector.x) * RADIANS_TO_DEGREES;

						//cycle it through another change in velocity to prevent bouncing again on the next update (it's still probably within the enemy on this update)
						bullet->x += bullet->dirVector.x * bullet->speed;
						bullet->y += bullet->dirVector.y * bullet->speed;

						break;
					}
					case(BT_SHOTGUN):
						enemy->hp -= BULLET_SHOTGUN_DMG;
						enemy->timeSinceDamaged = 0;	//update this var for hitflash to work

						//destroy bullet
						bullet->ttl = 0;
						break;

						//do nothing for enemy bullets
					}

					playSoundIsolated(SFX_ENEMY_HIT, SC_HITSOUNDS, false, bullet->x / SCREEN_WIDTH * 255.0);
				}

				enemy = enemy->next;
			}
		}
		else {
			//handling an enemy bullet
			//check for a collision with the player
			//only collide with the player if the player's not dead
			if (checkIntersection(bullet->collider, player->collider) && player->state != PS_DESTROYED) {
				//if there's a collision, apply damage to player
				if (player->iFrames <= 0) {
					player->iFrames = PLAYER_I_FRAMES_MAX;	//give i-frames
					--player->hp;

					//set background to do hurt flash
					background.backgroundFlashRedTimer = 0;

					playSound(SFX_PLAYER_HIT, SC_PLAYER, false, bullet->x / SCREEN_WIDTH * 255.0);
				}

				//bullet impact particle

				bullet->ttl = 0;	//destroy bullet
			}
		}

		//update ttl and destroy bullet if it's hit the end of ttl
		if (--bullet->ttl <= 0) {
			//create impact particle
			switch (bullet->type) {
				case(BT_NORMAL):	
					initParticle(initSpriteAnimated(app.gameplaySprites, 16, 1, 1, 1, SC_CENTER, 4, 0, 0.25, AL_ONESHOT), bullet->x, bullet->y, 0, 0, bullet->angle + 90, 1, NULL, biDraw);
					break;
				case(BT_ERRATIC):	
					initParticle(initSpriteAnimated(app.gameplaySprites, 16, 4, 1, 1, SC_CENTER, 5, 0, 0.25, AL_ONESHOT), bullet->x, bullet->y, 0, 0, bullet->angle + 45, 1, NULL, biDraw);
					break;
				case(BT_BOUNCER):	
					initParticle(initSpriteAnimated(app.gameplaySprites, 16, 7, 1, 1, SC_CENTER, 4, 0, 0.25, AL_ONESHOT), bullet->x, bullet->y, 0, 0, bullet->angle - 45, 1, NULL, biDraw);
					break;
				case(BT_SHOTGUN):	
					initParticle(initSpriteAnimated(app.gameplaySprites, 16, 10, 1, 1, SC_CENTER, 4, 0, 0.25, AL_ONESHOT), bullet->x, bullet->y, 0, 0, bullet->angle, 1, NULL, biDraw);
					break;
				case(BT_ENEMY):	
					initParticle(initSpriteAnimated(app.gameplaySprites, 16, 13, 1, 1, SC_CENTER, 4, 0, 0.25, AL_ONESHOT), bullet->x, bullet->y, 0, 0, bullet->angle + 90, 1, NULL, biDraw);
					break;
			}

			//edge case: last element
			if (bullet == stage.bulletTail) {
				//update bulletTail so it's not pointing to a freed element
				stage.bulletTail = prev;
			}
			
			if (bullet == stage.bulletHead) {
				//edge case: deleting first element
				stage.bulletHead = bullet->next;
				deleteBullet(bullet);
				bullet = prev = stage.bulletHead;	//update b and prev
				//no incrementation here
			} else {
				//general case
				prev->next = bullet->next;
				deleteBullet(bullet);
				bullet = prev->next;	//move b to next element
			}
		} else {
			//increment (must be in else, as deletion operations naturally increment)
			prev = bullet;
			bullet = bullet->next;
		}
	}
}

void drawBullets(void) {
	Bullet *bullet;

	for (bullet = stage.bulletHead; bullet != NULL; bullet = bullet->next) {
		//make player bullets semitransparent
		if (bullet->type != BT_ENEMY) {
			setTextureRGBA(bullet->sprite->atlas->texture, 255, 255, 255, 127);
			blitAndUpdateSpriteAnimatedEX(bullet->sprite, bullet->x, bullet->y, bullet->angle, NULL, SDL_FLIP_NONE);
			setTextureRGBA(bullet->sprite->atlas->texture, 255, 255, 255, 255);
		}
		else	//enemy bullet; draw as normal
			blitAndUpdateSpriteAnimatedEX(bullet->sprite, bullet->x, bullet->y, bullet->angle, NULL, SDL_FLIP_NONE);

		displayCollider(app.renderer, &COLOR_RED, bullet->collider);
	}
}

//no initialization function needed

//for deleting one bullet. frees dynamically allocated texture, then bullet itself.
static void deleteBullet(Bullet *bullet) {
	free(bullet->sprite);
	free(bullet);
	bullet = NULL;	//not really necessary but good practice
}

//for deleting all bullets.
void deleteBullets(void) {
	//clean linked list
	Bullet *bullet;

	bullet = stage.bulletHead;

	while (bullet != NULL) {
		//delete
		stage.bulletHead = bullet->next;
		deleteBullet(bullet);
		bullet = stage.bulletHead;
	}

	bullet = stage.bulletHead = stage.bulletTail = NULL;
}