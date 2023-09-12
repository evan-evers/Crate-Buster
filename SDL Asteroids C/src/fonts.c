#include "common.h"

#include "draw.h"
#include "fonts.h"
#include "stage.h"

extern App app;
extern Stage stage;

void drawText(char *text, int x, int y, SDL_Color color, TextAlignHorz horzAlign, int maxWidth);
void drawTextDropShadow(char *text, int x, int y, SDL_Color textColor, TextAlignHorz horzAlign, int maxWidth, SDL_Color shadowColor, int offset);
void findTextDimensions(char *text, int *w, int *h);

//fixed widths and heights for each grapheme in this bitmap font
static const int fontGraphemeWidth = 8;
static const int fontGraphemeHeight = 12;
static const int spaceBetweenGraphemes = 2;
static const int spaceBetweenLines = 4;
static const int cellWidth = 16;
static const int cellHeight = 16;

//draw text to the screen. will draw using the only font this game has in its assets.
//y describes top of text, x describes left, center, or right of text depending on horzAlign
//pass in NULL to maxWidth to draw without regards to maxWidth.
void drawText(char *text, int x, int y, SDL_Color color, TextAlignHorz horzAlign, int maxWidth) {
	SDL_Rect src, dest;	//source and destination rectangles for each grapheme

	//set text alignment
	if (horzAlign != TAH_LEFT) {
		//calculate length of string in pixels to align it to screen properly
		//it may seem inefficient to go through the whole string like this to find its length, 
		//but it's necessary, as the length of the string can easily change during runtime depending on what variables might be getting passed into it
		char *walker = text;
		int textWidthPixels = 0;
		while (*walker != '\0') {
			++textWidthPixels;
			++walker;
		}	
		textWidthPixels = textWidthPixels * (fontGraphemeWidth + spaceBetweenGraphemes);

		if(maxWidth != NULL)
			textWidthPixels = MIN(textWidthPixels, maxWidth);	//ensure that the text width doesn't exceed the maxWidth of the line

		if (horzAlign == TAH_CENTER) {
			x -= textWidthPixels * 0.5;
		}
		
		if (horzAlign == TAH_RIGHT){
			x -= textWidthPixels;
		}
	}

	//set dest's initial position
	dest.x = x;
	dest.y = y;
	dest.w = fontGraphemeWidth;
	dest.h = fontGraphemeHeight;

	//go through text string, blitting it to the screen

	//set text texture color
	setTextureRGBA(app.fontsAndUI->texture, color.r, color.g, color.b, color.a);

	while (*text != '\0') {
		//set src's position according to the character value
		src.x = (*text % 16) * cellWidth + 4;
		src.y = (*text / 16) * cellHeight + 2;
		src.w = fontGraphemeWidth;
		src.h = fontGraphemeHeight;

		SDL_RenderCopy(app.renderer, app.fontsAndUI->texture, &src, &dest);

		//increment dest's position so we don't render every grapheme on the same square
		if (maxWidth != NULL) {
			if (dest.x + fontGraphemeWidth + spaceBetweenGraphemes < maxWidth) {
				dest.x += fontGraphemeWidth + spaceBetweenGraphemes;
			} else {	//maxWidth exceeded; wrap text vertically
				dest.x = x;
				dest.y += fontGraphemeHeight + spaceBetweenLines;
			}
		}
		else {
			//no wrapping; just increment x
			dest.x += fontGraphemeWidth + spaceBetweenGraphemes;
		}

		//get next character
		++text;
	}

	//reset text texture color
	setTextureRGBA(app.fontsAndUI->texture, 255, 255, 255, 255);
}

//nice shortcut function for drawing text with a drop shadow, offset by "offset" pixels
void drawTextDropShadow(char *text, int x, int y, SDL_Color textColor, TextAlignHorz horzAlign, int maxWidth, SDL_Color shadowColor, int offset) {
	drawText(text, x + offset, y + offset, shadowColor, horzAlign, maxWidth);
	drawText(text, x, y, textColor, horzAlign, maxWidth);
}

//finds the width and height of a string of text, and returns it through w and h
void findTextDimensions(char *text, int *w, int *h) {
	//find width by finding the length of the string itself
	//this works because this game's only font is monospace
	char *walker = text;
	int textWidthPixels = 0;
	while (*walker != '\0') {
		++textWidthPixels;
		++walker;
	}
	//NULL checks allow the user to only grab width or height
	if (w != NULL)
		*w = textWidthPixels * (fontGraphemeWidth + spaceBetweenGraphemes);
	if (h != NULL)
		*h = fontGraphemeHeight;	//this game's only font has the same height for (almost) every grapheme
}