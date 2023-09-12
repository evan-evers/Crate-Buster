#ifndef WIDGETS_H
#define WIDGETS_H

#include "common.h"
#include "fonts.h"

/*
* Defines widgets used for controlling menus.
*/

//defines the different types of widgets there are
//shares a prefix 
typedef enum {
	WT_BUTTON,
	WT_SELECT,
	WT_SLIDER,
	WT_TEXT_INPUT,
	WT_CONTROL
} WidgetType;

typedef struct Widget Widget;

//general widget struct with void pointer for additional data per type
struct Widget {
	WidgetType     type;	
	char    name[MAX_LABEL_LENGTH];	//the name of the widget, which can be used to find it using getWidget (MUST BE UNIQUE TO ITS GROUP)
	char    groupName[MAX_LABEL_LENGTH];	//the name of the widget group this widget is in
	int     x;	//position and dimensions
	int     y;
	int     w;
	int     h;
	char    text[MAX_STRING_LENGTH];	//the text to be displayed for this within menus
	TextAlignHorz textAlignHorz;	//store the horizontal text alginment
	Widget *prev;	//stored as a doubly linked list
	Widget *next;
	void (*action)(void);	//the function to be called when this widget is clicked on or adjusted
	void (*data);			//additional data for most widget types
};

//a widget which allows the player to select from a number of strings (e.g. "English", "Espanol", "Francais")
typedef struct {
	int    numOptions;	//number of options the player can choose
	char **options;		//the options themselves, as an array strings
	int    x;	//position (independent of parent widget)
	int    y;
	int    value;		//index of currently selected option
} SelectWidget;

//a fader-style slider widget between a min and max value
typedef struct {
	int x;	//position and dimensions (independent of parent widget)
	int y;
	int w;
	int h;
	int value;	//value of the bar
	int minValue;	//min value of the bar
	int maxValue;	//max value of the bar
	int step;	//amount by which the bar is incremented per input
} SliderWidget;

//a widget for the user to input text by
//might be good to refactor this to work with pure controller input (via an onscreen virtual keyboard)
typedef struct {
	int x;	//position (independent of parent widget)
	int y;
	int maxLength;	//maximum length of the text the user can enter
	char *text;		//the text the user can enter
} TextInputWidget;

//a widget for keymapping
typedef struct {
	int x;	//position (independent of parent widget)
	int y;
	uint8_t value;	//holds the value of the last key or button pressed when this widget is active
} ControlWidget;

void initWidgets(void);
void updateWidgets(char *groupName);
void drawWidgets(char *groupName);
Widget *getWidget(char *name, char *groupName);
void deleteWidgets(void);
void closeWidgets(void);
void loadWidgets(char *filename);

void loadPreferences(void);
void savePreferences(void);

#endif