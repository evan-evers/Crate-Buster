#ifndef CURSOR_H
#define CURSOR_H

/*
* Handles custom cursor display and showing/hiding the cursor.
*/

//the state of the cursor (whether or not it should be drawn, what it should be drawn as)
typedef enum {
	CS_HIDE,
	CS_POINTER,
	CS_RETICLE
} CursorState;

void initCursor(void);
void drawCursor(void);
void deleteCursor(void);

#endif