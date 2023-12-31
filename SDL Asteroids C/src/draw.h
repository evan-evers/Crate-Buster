#ifndef DRAW_H
#define DRAW_H

/*
* Various structs and functions related to drawing stuff.
* This section needs some const pointers to avoid a lot of really inefficient copying
*/

//Contains the texture of a sprite atlas, plus relevant information related to it
typedef struct {
	SDL_Texture* texture;
	int w;	//width in pixels
	int h;	//height in pixels
} SpriteAtlas;

typedef enum {
	SC_TOP_LEFT,
	SC_TOP_CENTER,
	SC_TOP_RIGHT,
	SC_CENTER_LEFT,
	SC_CENTER,
	SC_CENTER_RIGHT,
	SC_BOTTOM_LEFT,
	SC_BOTTOM_CENTER,
	SC_BOTTOM_RIGHT
} SpriteCenter;

//A static, unanimated sprite from the sprite atlas.
typedef struct {
	SpriteAtlas* atlas;	//pointer to source
	int srcX;	//leftmost cell coordinate in cells
	int srcY;	//topmost cell coordinate in cells
	int w;		//width of one frame in cells
	int h;		//height of one frame in cells
	SpriteCenter center;	//determines where the center of the sprite is considered to be (a better name would probably be "origin")
} SpriteStatic;

//Specifies animation loop behavior.
//AL stands for animation loop.
typedef enum {
	AL_ONESHOT,			//play through the animation once, then freeze on the final frame
	AL_LOOP,			//loop animation indefinitely
	AL_BACK_AND_FORTH	//reverse animation direction when either end of the animation is reached
} AnimationLoop;

//A structure to keep track of important animation variables.
typedef struct {
	//sprite vars
	SpriteAtlas* atlas;	//pointer to source
	int srcX;	//leftmost cell coordinate in cells
	int srcY;	//topmost cell coordinate in cells
	int w;		//width of one frame in cells
	int h;		//height of one frame in cells
	SpriteCenter center;	//determines where the center of the sprite is considered to be (a better name would probably be "origin")

	//anim vars
	int frames;			//number of frames in the animation
	float currentFrame;	//keeps track of the frame the animation is currently on. Should be incremented by spd every frame until it exceeds sprite->frames.
	float spd;			//animation speed
	AnimationLoop loopBehavior;		//Defines how the animation loops
} SpriteAnimated;

void prepareScene(void);
void presentScene(void);
SDL_Texture* loadTexture(char* filename);
void setTextureRGBA(SDL_Texture *texture, int r, int g, int b, int a);
void blitSpriteStatic(const SpriteStatic *sprite, int x, int y);
void blitSpriteStaticEX(const SpriteStatic *sprite, int x, int y, float angle, const SDL_Point *origin, SDL_RendererFlip flip);
void blitAndUpdateSpriteAnimated(SpriteAnimated* sprite, int x, int y);
void blitAndUpdateSpriteAnimatedEX(SpriteAnimated* sprite, int x, int y, float angle, const SDL_Point* origin, SDL_RendererFlip flip);

//initializers (all use dynamic allocation, make sure to free())
SpriteAtlas* initSpriteAtlas(char* filename);
SpriteStatic *initSpriteStatic(const SpriteAtlas *atlas, int srcX, int srcY, int w, int h, SpriteCenter center);
SpriteAnimated *initSpriteAnimated(const SpriteAtlas *atlas, int srcX, int srcY, int w, int h, SpriteCenter center, int frames, float currentFrame, float spd, AnimationLoop loopBehavior);

//destructor for spriteAtlas (call free() on other structs)
void deleteSpriteAtlas(SpriteAtlas* atlas);

#endif