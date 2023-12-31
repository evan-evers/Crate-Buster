#include "common.h"

#include "SDL_image.h"

#include "draw.h"

extern App app;

void prepareScene(void);
void presentScene(void);
SDL_Texture *loadTexture(char *filename);
void setTextureRGBA(SDL_Texture *texture, int r, int g, int b, int a);
void blitSpriteStatic(const SpriteStatic* sprite, int x, int y);
void blitSpriteStaticEX(const SpriteStatic* sprite, int x, int y, float angle, const SDL_Point* origin, SDL_RendererFlip flip);
void blitAndUpdateSpriteAnimated(SpriteAnimated* sprite, int x, int y);
void blitAndUpdateSpriteAnimatedEX(SpriteAnimated* sprite, int x, int y, float angle, const SDL_Point* origin, SDL_RendererFlip flip);
SpriteAtlas *initSpriteAtlas(char *filename);
SpriteStatic *initSpriteStatic(const SpriteAtlas *atlas, int srcX, int srcY, int w, int h, SpriteCenter center);
SpriteAnimated *initSpriteAnimated(const SpriteAtlas *atlas, int srcX, int srcY, int w, int h, SpriteCenter center, int frames, float currentFrame, float spd, AnimationLoop loopBehavior);
void deleteSpriteAtlas(SpriteAtlas* atlas);

//Gets the scene ready for drawing
void prepareScene(void) {
	SDL_SetRenderDrawColor(app.renderer, 0x0, 0x0, 0x0, 0xff);
	SDL_RenderClear(app.renderer);
}

//Presents the drawn scene
void presentScene(void) {
	SDL_RenderPresent(app.renderer);
}

//loads a texture from a file
SDL_Texture *loadTexture(char* filename) {
	SDL_Texture *texture;

	//Debug message
	printf("Loading %s\n", filename);

	texture = IMG_LoadTexture(app.renderer, filename);

	return texture;
}

//set color and alpha modulation for a specific texture
//pass in 255 for the RGBA values to get the original texture
//wish I could just make a setSpriteRGBA function and overload it for static and animated sprites individually
void setTextureRGBA(SDL_Texture *texture, int r, int g, int b, int a) {
	SDL_SetTextureColorMod(texture, r, g, b);
	SDL_SetTextureAlphaMod(texture, a);
}

//Blit a sprite to the screen at the specified coordinates.
void blitSpriteStatic(const SpriteStatic* sprite, int x, int y) {
	SDL_Rect src;

	src.x = sprite->srcX;
	src.y = sprite->srcY;
	src.w = sprite->w;
	src.h = sprite->h;

	SDL_Rect dest;

	dest.w = sprite->w;
	dest.h = sprite->h;

	//blit the sprite with the specified center of orientation
	switch (sprite->center) {
		case(SC_TOP_LEFT):
			dest.x = x;
			dest.y = y;
		break;
		case(SC_TOP_CENTER):
			dest.x = x - dest.w * 0.5;
			dest.y = y;
		break;
		case(SC_TOP_RIGHT):
			dest.x = x - dest.w;
			dest.y = y;
			break;
		case(SC_CENTER_LEFT):
			dest.x = x;
			dest.y = y - dest.h * 0.5;
			break;
		case(SC_CENTER):
			dest.x = x - dest.w * 0.5;
			dest.y = y - dest.h * 0.5;
			break;
		case(SC_CENTER_RIGHT):
			dest.x = x - dest.w;
			dest.y = y - dest.h * 0.5;
			break;
		case(SC_BOTTOM_LEFT):
			dest.x = x;
			dest.y = y - dest.h;
			break;
		case(SC_BOTTOM_CENTER):
			dest.x = x - dest.w * 0.5;
			dest.y = y - dest.h;
			break;
		case(SC_BOTTOM_RIGHT):
			dest.x = x - dest.w;
			dest.y = y - dest.h;
			break;
	}

	SDL_RenderCopy(app.renderer, sprite->atlas->texture, &src, &dest);
}

//Blit a sprite to the screen at the specified coordinates, with rotation around an origin, flipping and alpha modulation.
//Pass NULL into the origin to rotate around the center of the destination rectangle.
void blitSpriteStaticEX(const SpriteStatic* sprite, int x, int y, float angle, const SDL_Point* origin, SDL_RendererFlip flip) {
	SDL_Rect src;

	src.x = sprite->srcX;
	src.y = sprite->srcY;
	src.w = sprite->w;
	src.h = sprite->h;

	SDL_Rect dest;

	dest.w = sprite->w;
	dest.h = sprite->h;

	//blit the sprite with the specified center of orientation
	switch (sprite->center) {
	case(SC_TOP_LEFT):
		dest.x = x;
		dest.y = y;
		break;
	case(SC_TOP_CENTER):
		dest.x = x - dest.w * 0.5;
		dest.y = y;
		break;
	case(SC_TOP_RIGHT):
		dest.x = x - dest.w;
		dest.y = y;
		break;
	case(SC_CENTER_LEFT):
		dest.x = x;
		dest.y = y - dest.h * 0.5;
		break;
	case(SC_CENTER):
		dest.x = x - dest.w * 0.5;
		dest.y = y - dest.h * 0.5;
		break;
	case(SC_CENTER_RIGHT):
		dest.x = x - dest.w;
		dest.y = y - dest.h * 0.5;
		break;
	case(SC_BOTTOM_LEFT):
		dest.x = x;
		dest.y = y - dest.h;
		break;
	case(SC_BOTTOM_CENTER):
		dest.x = x - dest.w * 0.5;
		dest.y = y - dest.h;
		break;
	case(SC_BOTTOM_RIGHT):
		dest.x = x - dest.w;
		dest.y = y - dest.h;
		break;
	}

	SDL_RenderCopyEx(app.renderer, sprite->atlas->texture, &src, &dest, angle, origin, flip);
}

//Blit a SpriteAnimated to the screen at the specified coordinates and update its animation.
void blitAndUpdateSpriteAnimated(SpriteAnimated* sprite, int x, int y) {
	//blitting
	SDL_Rect src;

	//this function supports animations done left-to-right in a single row
	//if the frames are greater than the edge of the sprite atlas being grabbed from, this function draws the first frame of the animation as a default
	//maybe at some point i'll add support for animations in multiple rows
	//>= may also be the incorrect comparison operator for this if statement; I'm assuming 0-indexed coordinated on the image
	if (sprite->srcX + sprite->w + ((int)sprite->currentFrame * sprite->w) >= sprite->atlas->w) {
		//Possible bug: edge of sprite atlas exceeded
		//The reason this is a possible bug and not a definite bug is because 
		//using a non-existent frame past the end of the atlas can be useful 
		//in situations where one is checking for the end of an animation
		printf("WARNING - 'blitAndUpdateSpriteAnimated' attempted to fetch pixels beyond the edge of its sprite atlas.\n");
		src.x = sprite->srcX;
	}
	else {
		//Normal case
		src.x = sprite->srcX + ((int)sprite->currentFrame * sprite->w);
	}
	src.y = sprite->srcY;
	src.w = sprite->w;
	src.h = sprite->h;

	SDL_Rect dest;

	dest.w = sprite->w;
	dest.h = sprite->h;

	//blit the sprite with the specified center of orientation
	switch (sprite->center) {
	case(SC_TOP_LEFT):
		dest.x = x;
		dest.y = y;
		break;
	case(SC_TOP_CENTER):
		dest.x = x - dest.w * 0.5;
		dest.y = y;
		break;
	case(SC_TOP_RIGHT):
		dest.x = x - dest.w;
		dest.y = y;
		break;
	case(SC_CENTER_LEFT):
		dest.x = x;
		dest.y = y - dest.h * 0.5;
		break;
	case(SC_CENTER):
		dest.x = x - dest.w * 0.5;
		dest.y = y - dest.h * 0.5;
		break;
	case(SC_CENTER_RIGHT):
		dest.x = x - dest.w;
		dest.y = y - dest.h * 0.5;
		break;
	case(SC_BOTTOM_LEFT):
		dest.x = x;
		dest.y = y - dest.h;
		break;
	case(SC_BOTTOM_CENTER):
		dest.x = x - dest.w * 0.5;
		dest.y = y - dest.h;
		break;
	case(SC_BOTTOM_RIGHT):
		dest.x = x - dest.w;
		dest.y = y - dest.h;
		break;
	}

	SDL_RenderCopy(app.renderer, sprite->atlas->texture, &src, &dest);

	//updating animation

	//progress animation
	sprite->currentFrame += sprite->spd;
	//loop behavior
	if (sprite->currentFrame >= sprite->frames || sprite->currentFrame < 0) {
		switch (sprite->loopBehavior) {
		case(AL_ONESHOT):
			//lock animation on the last frame
			sprite->currentFrame = sprite->frames - 1;
			break;
		case(AL_LOOP):
			//go back to start of animation
			sprite->currentFrame = 0;
			break;
		case(AL_BACK_AND_FORTH):
			sprite->currentFrame -= sprite->spd;	//make sure currentFrame stays within the bounds of the current animation
			sprite->spd = -sprite->spd;	//negate speed, inverting animation direction
			break;
		}
	}
}

//Blit a SpriteAnimated to the screen at the specified coordinates and update its animation. Supports rotation around an origin, flipping and alpha modulation.
//Pass NULL into the origin to rotate around the center of the destination rectangle.
void blitAndUpdateSpriteAnimatedEX(SpriteAnimated* sprite, int x, int y, float angle, const SDL_Point* origin, SDL_RendererFlip flip) {
	//blitting
	SDL_Rect src;

	//this function supports animations done left-to-right in a single row
	//if the frames are greater than the edge of the sprite atlas being grabbed from, this function draws the first frame of the animation as a default
	//maybe at some point i'll add support for animations in multiple rows
	//>= may also be the incorrect comparison operator for this if statement; I'm assuming 0-indexed coordinated on the image
	if (sprite->srcX + sprite->w + ((int)sprite->currentFrame * sprite->w) >= sprite->atlas->w) {
		//Possible bug: edge of sprite atlas exceeded
		printf("WARNING - 'blitAndUpdateSpriteAnimatedEX' attempted to fetch pixels beyond the edge of its sprite atlas.\n");
		src.x = sprite->srcX;
	}
	else {
		//Normal case
		src.x = sprite->srcX + ((int)sprite->currentFrame * sprite->w);
	}
	src.y = sprite->srcY;
	src.w = sprite->w;
	src.h = sprite->h;

	SDL_Rect dest;

	dest.w = sprite->w;
	dest.h = sprite->h;

	//blit the sprite with the specified center of orientation
	switch (sprite->center) {
	case(SC_TOP_LEFT):
		dest.x = x;
		dest.y = y;
		break;
	case(SC_TOP_CENTER):
		dest.x = x - dest.w * 0.5;
		dest.y = y;
		break;
	case(SC_TOP_RIGHT):
		dest.x = x - dest.w;
		dest.y = y;
		break;
	case(SC_CENTER_LEFT):
		dest.x = x;
		dest.y = y - dest.h * 0.5;
		break;
	case(SC_CENTER):
		dest.x = x - dest.w * 0.5;
		dest.y = y - dest.h * 0.5;
		break;
	case(SC_CENTER_RIGHT):
		dest.x = x - dest.w;
		dest.y = y - dest.h * 0.5;
		break;
	case(SC_BOTTOM_LEFT):
		dest.x = x;
		dest.y = y - dest.h;
		break;
	case(SC_BOTTOM_CENTER):
		dest.x = x - dest.w * 0.5;
		dest.y = y - dest.h;
		break;
	case(SC_BOTTOM_RIGHT):
		dest.x = x - dest.w;
		dest.y = y - dest.h;
		break;
	}

	SDL_RenderCopyEx(app.renderer, sprite->atlas->texture, &src, &dest, angle, origin, flip);

	//updating animation

	//progress animation
	sprite->currentFrame += sprite->spd;
	//loop behavior
	if (sprite->currentFrame >= sprite->frames || sprite->currentFrame < 0) {
		switch (sprite->loopBehavior) {
		case(AL_ONESHOT):
			//lock animation on the last frame
			sprite->currentFrame = sprite->frames - 1;
			break;
		case(AL_LOOP):
			//go back to start of animation
			sprite->currentFrame = 0;
			break;
		case(AL_BACK_AND_FORTH):
			sprite->currentFrame -= sprite->spd;	//make sure currentFrame stays within the bounds of the current animation
			sprite->spd = -sprite->spd;	//negate speed, inverting animation direction
			break;
		}
	}
}



//initializers and destructors
//all use dynamic allocation, make sure to use free() and destructors
//srcX, srcY, w and h are all in grid coordinates, not pixels

//loads the atlas's texture and initializes its members
SpriteAtlas* initSpriteAtlas(char* filename) {
	SpriteAtlas *atlas = calloc(1, sizeof(SpriteAtlas));
	atlas->texture = loadTexture(filename);

	if (atlas->texture == NULL) {
		printf("ERROR - File '%s' could not be loaded: %s\n", filename, IMG_GetError());
		return NULL;
	}

	//initialize w and h
	SDL_QueryTexture(atlas->texture, NULL, NULL, &atlas->w, &atlas->h);

	return atlas;
}

//initializes struct's members
SpriteStatic* initSpriteStatic(const SpriteAtlas* atlas, int srcX, int srcY, int w, int h, SpriteCenter center) {
	SpriteStatic* sprite = calloc(1, sizeof(SpriteStatic));
	sprite->atlas = atlas;
	//convert these bits into pixels now for quicker calculations later
	sprite->srcX = srcX * SPRITE_ATLAS_CELL_W;
	sprite->srcY = srcY * SPRITE_ATLAS_CELL_H;
	sprite->w = w * SPRITE_ATLAS_CELL_W;
	sprite->h = h * SPRITE_ATLAS_CELL_H;
	sprite->center = center;

	return sprite;
}

//initializes struct's members
//current frame should be set to 0 unless you want to start the animation after its beginning
SpriteAnimated* initSpriteAnimated(const SpriteAtlas* atlas, int srcX, int srcY, int w, int h, SpriteCenter center, int frames, float currentFrame, float spd, AnimationLoop loopBehavior) {
	SpriteAnimated* sprite = calloc(1, sizeof(SpriteAnimated));
	sprite->atlas = atlas;
	//convert these bits into pixels now for quicker calculations later
	sprite->srcX = srcX * SPRITE_ATLAS_CELL_W;
	sprite->srcY = srcY * SPRITE_ATLAS_CELL_H;
	sprite->w = w * SPRITE_ATLAS_CELL_W;
	sprite->h = h * SPRITE_ATLAS_CELL_W;
	sprite->center = center;

	sprite->frames = frames;
	sprite->currentFrame = currentFrame;
	sprite->spd = spd;
	sprite->loopBehavior = loopBehavior;

	return sprite;
}

//destructor for spriteAtlas (call free() on other structs)
void deleteSpriteAtlas(SpriteAtlas* atlas) {
	SDL_DestroyTexture(atlas->texture);
	free(atlas);
	atlas = NULL;	//clean up dangling pointer
}