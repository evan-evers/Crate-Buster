#include "common.h"

#include "cursor.h"
#include "draw.h"
#include "input.h"

extern App app;
extern InputManager input;

static SpriteStatic* sprCursorPoint;
static SpriteStatic* sprCursorReticle;

//very straightforward functions.

void initCursor(void) {
	//hide cursor
	SDL_ShowCursor(SDL_DISABLE);

	//initialize variables
	sprCursorPoint = initSpriteStatic(app.fontsAndUI, 7, 19, 1, 1, SC_TOP_LEFT);
	sprCursorReticle = initSpriteStatic(app.fontsAndUI, 7, 20, 1, 1, SC_CENTER);
}

void drawCursor(void) {
	//only display cursor if user is using mouse and keyboard
	if (input.lastControllerType == LCT_KEYBOARD_AND_MOUSE) {
		switch (app.cursorState) {
		case(CS_POINTER):
			blitSpriteStatic(sprCursorPoint, input.mouse.x - 3, input.mouse.y - 3);
			break;
		case(CS_RETICLE):
			blitSpriteStatic(sprCursorReticle, input.mouse.x, input.mouse.y);
			break;
		}
	}
}

void deleteCursor(void) {
	free(sprCursorPoint);
	free(sprCursorReticle);
}