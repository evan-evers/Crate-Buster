#include "common.h"

#include "particles.h"
#include "stage.h"

extern Stage stage;

Particle *initParticle(SpriteAnimated *sprite, float x, float y, float deltaX, float deltaY, float angle, int ttl, void (*update)(void), void (*draw)(void));
void updateParticles(void);
void drawParticles(void);
static void deleteParticle(Particle *particle);
void deleteParticles(void);
void explosionDraw(Particle *particle);

//create a particle and add it to the stage's particle list
Particle *initParticle(SpriteAnimated *sprite, float x, float y, float deltaX, float deltaY, float angle, int ttl, void (*update)(void), void (*draw)(void)) {
	Particle *particle;

	particle = calloc(1, sizeof(Particle));
	if (stage.particleHead == NULL) {
		stage.particleHead = stage.particleTail = particle;
	}
	else {
		stage.particleTail->next = particle;
		stage.particleTail = particle;
	}

	particle->x = x;
	particle->y = y;
	particle->deltaX = deltaX;
	particle->deltaY = deltaY;
	particle->angle = angle;
	particle->ttl = ttl;
	particle->sprite = sprite;
	particle->update = update;
	particle->draw = draw;
	particle->next = NULL;

	return particle;
}

//update particles according to their update functions
void updateParticles(void) {
	Particle *particle, *prev;
	particle = prev = stage.particleHead;

	while (particle != NULL) {
		//call particle's update function
		if(particle->update != NULL)
			particle->update(particle);

		//delete a particle if its ttl is <= 0
		//user has control over whether ttl gets decremented or not
		if (particle->ttl <= 0) {
			//edge case: last element
			if (particle == stage.particleTail) {
				//update bulletTail so it's not pointing to a freed element
				stage.particleTail = prev;
			}

			if (particle == stage.particleHead) {
				//edge case: deleting first element
				stage.particleHead = particle->next;
				deleteParticle(particle);
				particle = prev = stage.particleHead;	//update b and prev
				//no incrementation here
			} else {
				//general case
				prev->next = particle->next;
				deleteParticle(particle);
				particle = prev->next;	//move b to next element
			}
		} else {
			//increment (must be in else, as deletion operations naturally increment)
			prev = particle;
			particle = particle->next;
		}
	}
}

//draw particles according to their draw functions
void drawParticles(void) {
	Particle *particle = stage.particleHead;

	while (particle != NULL) {
		if (particle->draw != NULL)
			particle->draw(particle);

		particle = particle->next;
	}
}

//deletes a single particle
//this is static void b/c a user should definitely not have access to this; it'd really mess up linked lists
static void deleteParticle(Particle *particle) {
	free(particle->sprite);
	free(particle);
	particle = NULL;
}

//deletes all particles in the stage's linked list
void deleteParticles(void) {
	Particle *particle = stage.particleHead;

	while (particle != NULL) {
		stage.particleHead = particle->next;
		deleteParticle(particle);
		particle = stage.particleHead;
	}

	stage.particleHead = stage.particleTail = NULL;
}



//specific particle draw functions shared between classes

//explosion particle for crates, player, and enemies
void explosionDraw(Particle *particle) {
	//draw if the animation isn't over yet (using extra frame trick to tell when animation is done)
	if (particle->sprite->currentFrame < particle->sprite->frames - 1)
		blitAndUpdateSpriteAnimatedEX(particle->sprite, particle->x, particle->y, particle->angle, NULL, SDL_FLIP_NONE);
	else
		particle->ttl = 0; 	//delete particle when animation is over
}