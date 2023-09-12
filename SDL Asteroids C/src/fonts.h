#ifndef FONTS_H
#define FONTS_H

/*
* Header file for bitmap font implementation.
* Future features:
*	-Base implementation off of TTF rather than a bitmap
*	-Support for multiple fonts in the same game
*	-Unicode support (for localization support)
*	-Read all text from files and none from literals (for localization support)
*	-Vertical alginment support in addition to horizontal alignment support
*	-Add support for sprites within strings
*/

//horizontal text alignment enum
typedef enum {
	TAH_LEFT,
	TAH_CENTER,
	TAH_RIGHT
} TextAlignHorz;

void drawText(char *text, int x, int y, SDL_Color color, TextAlignHorz horzAlign, int maxWidth);
void drawTextDropShadow(char *text, int x, int y, SDL_Color textColor, TextAlignHorz horzAlign, int maxWidth, SDL_Color shadowColor, int offset);
void findTextDimensions(char *text, int *w, int *h);

#endif