#include "common.h"

#include <float.h>	//for the float max

#include "./json/cJSON.h"
#include "fonts.h"
#include "input.h"
#include "sound.h"
#include "utility.h"
#include "widgets.h"

extern App app;
extern InputManager input;

//linked list holding widgets is static to this file, rather than being a part of any struct
static Widget *widgetHead;
static Widget *widgetTail;

static void createWidget(cJSON *root);
static WidgetType getWidgetType(char *type);
static void createButtonWidget(Widget *w, cJSON *root);
static void createSelectWidget(Widget *w, cJSON *root);
static void createSliderWidget(Widget *w, cJSON *root);
static void createTextInputWidget(Widget *w, cJSON *root);
static void createControlWidget(Widget *w, cJSON *root);
static void focusTextInputWidget(void);
//static void updateControlWidget(void);
static void drawButtonWidget(Widget *w);
static void drawSelectWidget(Widget *w);
static void drawSliderWidget(Widget *w);
static void drawTextInputWidget(Widget *w);
//static void drawControlWidget(Widget *w);
static void changeWidgetValue(int dir);
static void changeSliderValueMouse();

/*
* TODO:
*	-Implement control binding widget (for a future project)
*/

static const int SELECTION_ARROW_OFFSET = 20;
static const int CURSOR_BLINK_OFFSET = 0;

static int cursorBlink;	//timer for doing cursor blink on the text input widget
bool focusInputWidget;	//when set to true, updateWidgets will focus on text input (not static to allow highscores menu to bypass this
static bool focusControlWidget;	//when set to true, updateWidgets will focus on grabbing an input for keybinding/button binding
Widget *lastActiveWidget;		//keep track of the last active widget for sound clicks when navigating menu with mouse
SpriteAnimated *selectionArrow;		//arrow indicating which menu option is currently selected
SpriteStatic *sliderBarWhite;	//bar graphic for the selected slider
SpriteStatic *sliderBarBlue;	//bar graphic for a non-selected slider

//initializes important widget variables
void initWidgets(void) {
	memset(&widgetHead, 0, sizeof(Widget));
	widgetTail = widgetHead = NULL;

	cursorBlink = 0;
	focusInputWidget = false;
	focusControlWidget = false;
	selectionArrow = initSpriteAnimated(app.fontsAndUI, 4, 18, 1, 1, SC_TOP_LEFT, 4, 0, 0.12, AL_LOOP);
	sliderBarWhite = initSpriteStatic(app.fontsAndUI, 0, 21, 6, 1, SC_TOP_LEFT);
	sliderBarBlue = initSpriteStatic(app.fontsAndUI, 0, 22, 6, 1, SC_TOP_LEFT);
}

//update function for widgets
//handles moving between widgets and editing their values
//pass in a group name to update only that group of widgets, or NULL to update all widgets
void updateWidgets(char *groupName) {
	//update aesthetic variables
	++cursorBlink;

	if (!focusInputWidget && !focusControlWidget) {
		//handle widgets normally, switching between active and inactive ones

		//handle moving between widgets
		//the logic for this changes significantly between button movement and mouse movement,
		//so both are broken up into their own respective things
		if (input.mouseWasMoved) {
			float minYDistance = FLT_MAX;

			//find the widget that the mouse is vertically closest to
			for (Widget *widget = widgetHead; widget != NULL; widget = widget->next) {
				if (groupName == NULL || strcmp(widget->groupName, groupName) == 0) {
					if (minYDistance > abs(widget->y - input.mouse.y)) {
						app.activeWidget = widget;
						minYDistance = abs(widget->y - input.mouse.y);
					}
				}
			}

			if(app.activeWidget != lastActiveWidget)
				playSoundIsolated(SFX_CLICK, SC_UI, false, PAN_CENTER);
		} else {
			if (input.upPressed > 0) {
				input.upPressed = 0;

				//the do-while is here to move to the next widget in the current group, rather to a widget which could potentially be in a different group
				do {
					//move one element "left"
					app.activeWidget = app.activeWidget->prev;

					//moved past the "left" end of the list; loop around
					if (app.activeWidget == NULL) {
						app.activeWidget = widgetTail;
					}
				} while (groupName != NULL && strcmp(app.activeWidget->groupName, groupName) != 0);

				//play a click sound
				playSoundIsolated(SFX_CLICK, SC_UI, false, PAN_CENTER);
			}

			if (input.downPressed > 0) {
				input.downPressed = 0;

				//the do-while is here to move to the next widget in the current group, rather to a widget which could potentially be in a different group
				do {
					//move one element "right"
					app.activeWidget = app.activeWidget->next;

					//moved past the "right" end of the list; loop around
					if (app.activeWidget == NULL) {
						app.activeWidget = widgetHead;
					}
				} while (groupName != NULL && strcmp(app.activeWidget->groupName, groupName) != 0);

				//play a click sound
				playSoundIsolated(SFX_CLICK, SC_UI, false, PAN_CENTER);
			}
		}

		//change values of select and slider widgets
		if (input.leftPressed > 0) {
			input.leftPressed = 0;

			changeWidgetValue(-1);
		}
		if (input.rightPressed > 0) {
			input.rightPressed = 0;

			changeWidgetValue(1);
		}

		//perform a widget's action when the fire button is pressed
		if (input.firePressed > 0) {
			input.firePressed = 0;

			if (app.activeWidget->type == WT_TEXT_INPUT) {
				//reset cursor blink for aesthetic purposes
				cursorBlink = 0;

				//focus on getting text input
				focusInputWidget = true;

				memset(input.inputText, 0, sizeof(input.inputText));
			} else if (app.activeWidget->type == WT_CONTROL) {
				//make sure the control widget doesn't instantly get whatever was pressed to enter this state
				input.lastKeyPressed = -1;
				input.lastMouseButtonPressed = -1;
				input.lastGamepadButtonPressed = -1;

				focusControlWidget = true;

			} else if (app.activeWidget->action != NULL) {
				switch (app.activeWidget->type) {
				case(WT_BUTTON):
					//do whatever the widget's action function is
					app.activeWidget->action();
					break;
				case(WT_SELECT):
					//move forward one option
					changeWidgetValue(1);
					break;
				case(WT_SLIDER):
					//handled below (line 181)
					break;
				}
			}
		}

		//mouse slider stuff (checks fire, not firePressed, to allow for continuous drag)
		if (input.fire > 0) {
			input.fire = 0;

			if (app.activeWidget->type == WT_SLIDER) {
				changeSliderValueMouse();
			}
		}
	} else if (focusInputWidget) {
		//focus only on getting text input
		focusTextInputWidget();
	} else if (focusControlWidget) {
		//focus only on changing a control binding
		//focusControlWidget();
	}

	lastActiveWidget = app.activeWidget;
}

//draws all widgets currently in the list
//pass in NULL as the groupName to draw every widget
void drawWidgets(char *groupName) {
	for (Widget *widget = widgetHead; widget != NULL; widget = widget->next) {
		//only draw widgets in the current group
		if (groupName == NULL || strcmp(groupName, widget->groupName) == 0) {
			switch (widget->type) {
			case WT_BUTTON:
				drawButtonWidget(widget);
				break;

			case WT_SELECT:
				drawSelectWidget(widget);
				break;

			case WT_SLIDER:
				drawSliderWidget(widget);
				break;

			case WT_TEXT_INPUT:
				drawTextInputWidget(widget);
				break;

			case WT_CONTROL:
				//drawControlWidget(widget);
				break;

			default:
				break;
			}

			//draw selection arrow
			if (widget == app.activeWidget) {
				switch (widget->textAlignHorz) {
				case(TAH_LEFT):
					blitAndUpdateSpriteAnimated(selectionArrow, widget->x - SELECTION_ARROW_OFFSET, widget->y - 2);
					break;
				case(TAH_CENTER):
					blitAndUpdateSpriteAnimated(selectionArrow, widget->x - widget->w * 0.5 - SELECTION_ARROW_OFFSET, widget->y - 2);
					break;
				case(TAH_RIGHT):
					blitAndUpdateSpriteAnimated(selectionArrow, widget->x - widget->w - SELECTION_ARROW_OFFSET, widget->y - 2);
					break;
				}
			}
		}
	}
}

//gets a specific widget from the list based off of its name and group name
Widget *getWidget(char *name, char *groupName) {
	Widget *w;

	for (w = widgetHead; w != NULL; w = w->next) {
		if (strcmp(w->name, name) == 0 && strcmp(w->groupName, groupName) == 0) {
			return w;
		}
	}

	printf("WARNING: No such widget: name='%s', groupName='%s'", name, groupName);

	return NULL;
}

//deletes all widgets currently in the static linked list
//call this when you want to unload some widgets, NOT when you want to close out the widget system
void deleteWidgets(void) {
	Widget *widget = widgetHead;

	while (widget != NULL) {
		widgetHead = widgetHead->next;	//use widgetHead to store a pointer to the next widget
		//I could create seperate functions for deleting different types, like in the creation function.
		//but I don't want to. this is fine.
		switch (widget->type) {
		case WT_BUTTON:
			free(widget);
			break;

		case WT_SELECT:
			;	//empty statement to make the compiler happy
			//deallocate using a SelectWidget pointer rather than a void pointer, for proper destruction of the object
			SelectWidget *selectWidget = (SelectWidget *)widget->data;
			//deallocate options strings
			for (int i = 0; i < selectWidget->numOptions; ++i)
				free(selectWidget->options[i]);
			free(selectWidget->options);	//deallocate array of pointers to options strings
			free(selectWidget); //deallocate subwidget
			free(widget); //deallocate parent widget
			break;

		case WT_SLIDER:
			;	//empty statement to make the compiler happy
			//deallocate using a SliderWidget pointer rather than a void pointer, for proper destruction of the object
			SliderWidget *sliderWidget = (SliderWidget *)widget->data;
			free(sliderWidget); //deallocate subwidget
			free(widget); //deallocate parent widget
			break;

		case WT_TEXT_INPUT:
			;	//empty statement to make the compiler happy
			//deallocate using a TextInputWidget pointer rather than a void pointer, for proper destruction of the object
			TextInputWidget *textInputWidget = (TextInputWidget *)widget->data;
			free(textInputWidget->text);	//free text buffer
			free(textInputWidget); //deallocate subwidget
			free(widget);	//deallocate parent widget
			break;

		case WT_CONTROL:
			;	//empty statement to make the compiler happy
			//deallocate using a ControlWidget pointer rather than a void pointer, for proper destruction of the object
			ControlWidget *controlWidget = (ControlWidget *)widget->data;
			free(controlWidget); //deallocate subwidget
			free(widget);	//deallocate parent widget
			break;
		}
		
		widget = widgetHead;
	}
}

//close out the widget system
//deletes all widgets AND sprites needed for the widget system to run
//should probably only be called when the game is being closed
void closeWidgets(void) {
	deleteWidgets();

	//free sprites
	free(selectionArrow);
	free(sliderBarWhite);
	free(sliderBarBlue);
}

//loads a set of widgets from a JSON file
//notably, this function does not set a widget's "action" function pointer
//which maybe defeats the whole purpose of loading them from a JSON file since you have to initialize at least part of it programmatically
//also you have no access to important positional variables like camera position or screen width and height from within a JSON file
//TODO: look into initializing widgets purely programmatically from a function, instead of this half-JSON half-programmatic weirdness
void loadWidgets(char *filename) {
	cJSON *root, *node;
	char *text;

	//get JSON as text
	text = readFile(filename);

	//translate JSON text into a cJSON object
	root = cJSON_Parse(text);

	//widgets are stored as a cJSON array; iterate through that array
	//"root" is a cJSON array of cJSON objects (the widgets themselves)
	//a cJSON array is accessed as a linked list pointed to by (array name)->child (in this case, root->child)
	for (node = root->child; node != NULL; node = node->next) {
		createWidget(node);
	}

	//clean up cJSON object and text buffer
	cJSON_Delete(root);
	free(text);

	//avoid a click sound when widgets load in
	lastActiveWidget = app.activeWidget;
}

//calls the appropriate widget creation function according to the widget's type
static void createWidget(cJSON *root) {
	Widget *widget;
	int     type;

	//figure out the widget's type and process it accordingly
	type = getWidgetType(cJSON_GetObjectItem(root, "type")->valuestring);

	if (type != -1) {
		//allocate 0ed memory for widget, and add it to the widget list
		widget = calloc(1, sizeof(Widget));
		if (widgetHead == NULL) {
			widgetHead = widget;
			widgetTail = widget;
			//no next or previous element
			widget->next = NULL;
			widget->prev = NULL;
		} else {
			widgetTail->next = widget;	//this line takes care of setting widget->next for the previous element
			widget->prev = widgetTail;	//set prev pointer on tail element
			widgetTail = widget;
		}

		//initialize variables common to all widgets
		STRNCPY(widget->name, cJSON_GetObjectItem(root, "name")->valuestring, MAX_LABEL_LENGTH);
		STRNCPY(widget->groupName, cJSON_GetObjectItem(root, "groupName")->valuestring, MAX_LABEL_LENGTH);
		STRNCPY(widget->text, cJSON_GetObjectItem(root, "text")->valuestring, MAX_STRING_LENGTH);
		widget->type = getWidgetType(cJSON_GetObjectItem(root, "type")->valuestring);
		widget->textAlignHorz = getWidgetTextAlignHorz(cJSON_GetObjectItem(root, "textAlignHorz")->valuestring);
		widget->x = cJSON_GetObjectItem(root, "x")->valueint;
		widget->y = cJSON_GetObjectItem(root, "y")->valueint;

		//initialize variables specific to this widget's type
		switch (widget->type) {
		case WT_BUTTON:
			createButtonWidget(widget, root);
			break;

		case WT_SELECT:
			createSelectWidget(widget, root);
			break;

		case WT_SLIDER:
			createSliderWidget(widget, root);
			break;

		case WT_TEXT_INPUT:
			createTextInputWidget(widget, root);
			break;

		case WT_CONTROL:
			createControlWidget(widget, root);
			break;

		//error case
		default:
			printf("WARNING - Attempted to create unknown widget type.\n");
			break;
		}
	}
}

//type-specific widget creation functions
//these only handle initializing things specific to their own widget type
//variables common to all widget types are initialized in createWidgets

static void createButtonWidget(Widget *widget, cJSON *root) {
	//give the widget the same dimensions as its text
	findTextDimensions(widget->text, &widget->w, &widget->h);

	widget->data = NULL;
}

static void createSelectWidget(Widget *widget, cJSON *root) {
	cJSON *options, *node;	//pointers to parse the selectWidget's JSON
	int           i, length;
	SelectWidget *selectWidget;	//temp pointer to initialize selectWidget

	//allocate 0ed memory for subwidget, and set its parent widget to point to it
	selectWidget = calloc(1, sizeof(SelectWidget));
	widget->data = selectWidget;

	//initialize select widget

	//initialize numOptions
	options = cJSON_GetObjectItem(root, "options");
	selectWidget->numOptions = cJSON_GetArraySize(options);

	if (selectWidget->numOptions > 0) {
		//allocate space for selectWidget's array of options (an array of strings)
		selectWidget->options = malloc(sizeof(char *) * selectWidget->numOptions);

		i = 0;	//an index to go through the elements of selectWidget->options

		//allocate and initialize each option
		//"options" is a cJSON array, and thus the array is stored as a linked list pointed to by "options->child"
		for (node = options->child; node != NULL; node = node->next) {
			length = strlen(node->valuestring) + 1;		//TODO: check if this needs to be set to +1 (cJSON valuestrings are null terminated, so I think the answer is no)

			selectWidget->options[i] = malloc(length);

			STRNCPY(selectWidget->options[i], node->valuestring, length);

			++i;	//since the for loop iterates through a linked list, this seperate iterator for "selectWidget->options" must be incremented here
		}
	}

	//give the parent widget the same dimensions as its text
	findTextDimensions(widget->text, &widget->w, &widget->h);

	//set selectWidget's position relative to its parent widget
	selectWidget->x = widget->x + widget->w + 50;
	selectWidget->y = widget->y;
}

static void createSliderWidget(Widget *widget, cJSON *root) {
	SliderWidget *sliderWidget;

	//allocate 0ed memory for subwidget, and set its parent widget to point to it
	sliderWidget = calloc(1, sizeof(SliderWidget));
	widget->data = sliderWidget;

	//set variables related to how the bar can be changed by the user
	sliderWidget->minValue = cJSON_GetObjectItem(root, "minValue")->valueint;
	sliderWidget->maxValue = cJSON_GetObjectItem(root, "maxValue")->valueint;
	sliderWidget->step = cJSON_GetObjectItem(root, "step")->valueint;


	//give the parent widget the same dimensions as its text
	findTextDimensions(widget->text, &widget->w, &widget->h);

	//set subwidget position and dimensions
	sliderWidget->x = widget->x + widget->w + 50;
	sliderWidget->y = widget->y;
	sliderWidget->w = 94;	//TODO: set this up to be based on the sprite
	sliderWidget->h = 14;
}

static void createTextInputWidget(Widget *widget, cJSON *root) {
	TextInputWidget *textInputWidget;

	//allocate 0ed memory for subwidget, and set its parent widget to point to it
	textInputWidget = calloc(1, sizeof(TextInputWidget));
	memset(textInputWidget, 0, sizeof(TextInputWidget));
	widget->data = textInputWidget;

	//set max length of the text string that can be input, then malloc that amount of space for the text input
	textInputWidget->maxLength = cJSON_GetObjectItem(root, "maxLength")->valueint;
	textInputWidget->text = malloc(sizeof(char) * (textInputWidget->maxLength + 1));
	textInputWidget->text[0] = '\0';

	//give the parent widget the same dimensions as its text
	findTextDimensions(widget->text, &widget->w, &widget->h);

	//set position relative to parent widget
	textInputWidget->x = widget->x + widget->w;
}

static void createControlWidget(Widget *widget, cJSON *root) {
	ControlWidget *controlWidget;

	//allocate space for subwidget, 0 subwidget's memory, and set its parent widget to point to it
	controlWidget = calloc(1, sizeof(ControlWidget));
	widget->data = controlWidget;

	//give the parent widget the same dimensions as its text
	findTextDimensions(widget->text, &widget->w, &widget->h);
}

//handle a text input widget's text input when the widget is pressed
static void focusTextInputWidget(void) {
	TextInputWidget *textInputWidget = (TextInputWidget *)app.activeWidget->data;
	int	currentTextLength = strnlen(textInputWidget->text, textInputWidget->maxLength),		//get length of text already entered
		//get length of text the user has entered this frame
		inputTextLength = strnlen(input.inputText, MAX_INPUT_LENGTH);

	//handle backspace
	//this could use an input abstraction
	if (currentTextLength > 0 && input.backspacePressed > 0) {
		input.backspacePressed = 0;	//reset backspace to make sure it doesn't activate on the next frame

		--currentTextLength;	//decrement current text length, as we're removing a character
		textInputWidget->text[currentTextLength] = '\0';	//remove a character from the end of the string
	}

	//clamp length of text the user has entered this frame to ensure the user can't enter more characters than the text input widget allows for
	//the - 1 is to account for the null terminator
	if (inputTextLength + currentTextLength > textInputWidget->maxLength)
		inputTextLength = textInputWidget->maxLength - currentTextLength;
	
	//append input.inputText onto textInputWidget->text
	if (inputTextLength > 0) {
		strncat(textInputWidget->text, input.inputText, inputTextLength);

		//clear input text
		input.inputText[0] = '\0';
	}

	//handle exiting the widget
	if (input.firePressed > 0) {
		input.firePressed = 0;

		focusInputWidget = false;	//unfocus from this widget

		//call this widget's action function
		if (app.activeWidget->action != NULL)
			app.activeWidget->action();
	}
}

//gets a widget's type from its JSON data and returns it as a string
//this is a very simple text to enum conversion function
static int getWidgetType(char *type) {
	//expected cases
	if (strcmp(type, "WT_BUTTON") == 0) {
		return WT_BUTTON;
	} else if (strcmp(type, "WT_SELECT") == 0) {
		return WT_SELECT;
	} else if (strcmp(type, "WT_SLIDER") == 0) {
		return WT_SLIDER;
	} else if (strcmp(type, "WT_TEXT_INPUT") == 0) {
		return WT_TEXT_INPUT;
	} else if (strcmp(type, "WT_CONTROL") == 0) {
		return WT_CONTROL;
	}

	//error case: unknown widget type
	return -1;
}

//text to enum converter that gets a widget's horizontal text alignment
static TextAlignHorz getWidgetTextAlignHorz(char *textAlignHorz) {
	//expected cases
	if (strcmp(textAlignHorz, "TAH_LEFT") == 0) {
		return TAH_LEFT;
	} else if (strcmp(textAlignHorz, "TAH_CENTER") == 0) {
		return TAH_CENTER;
	} else if (strcmp(textAlignHorz, "TAH_RIGHT") == 0) {
		return TAH_RIGHT;
	}

	//error case: unknown widget type
	return -1;
}

//changes the value of a widget based on its type
//only changes values for select and slider widgets
static void changeWidgetValue(int dir) {
	SelectWidget *select;
	SliderWidget *slider;

	//process change according to its widget type
	switch (app.activeWidget->type) {
	case WT_SELECT:
		//get a pointer to the subwidget of the current widget
		select = (SelectWidget *)app.activeWidget->data;

		//change value
		select->value += dir;

		//value wraparound
		if (select->value < 0) {
			select->value = select->numOptions - 1;
		}
		if (select->value >= select->numOptions) {
			select->value = 0;
		}

		//invoke the widget's action, if it has one
		if (app.activeWidget->action != NULL) {
			app.activeWidget->action();
		}

		//play a click sound
		playSoundIsolated(SFX_CLICK, SC_UI, false, PAN_CENTER);

		break;

	case WT_SLIDER:
		//get a pointer to the subwidget of the current widget
		slider = (SliderWidget *)app.activeWidget->data;

		//change slider, clamping its value between its max and min
		slider->value = MIN(MAX(slider->value + (slider->step * dir), slider->minValue), slider->maxValue);

		//invoke the widget's action, if it has one
		if (app.activeWidget->action != NULL) {
			app.activeWidget->action();
		}

		//play a click sound
		playSoundIsolated(SFX_CLICK, SC_UI, false, PAN_CENTER);

		break;

	default:
		//do nothing if the user presses left or right on a button, text input or control binding widget
		break;
	}
}

//a function specifically for changing slider values with the mouse
static void changeSliderValueMouse() {
	SelectWidget *select;
	SliderWidget *slider;

	//get a pointer to the subwidget of the current widget
	slider = (SliderWidget *)app.activeWidget->data;

	//check if mouse is around the slider
	//change slider according to mouse x value relative to slider width (adjusted for aesthetic elements)
	int newValue = ((input.mouse.x - slider->x) / (float)(slider->w - 4)) * (slider->maxValue - slider->minValue);

	//only do widget action and play sound if a new value was input and the new input is within the bounds of the slider
	if (newValue >= slider->minValue && newValue <= slider->maxValue && newValue != slider->value) {
		slider->value = newValue;

		//invoke the widget's action, if it has one
		if (app.activeWidget->action != NULL) {
			app.activeWidget->action();
		}

		//play a click sound
		playSoundIsolated(SFX_CLICK, SC_UI, false, PAN_CENTER);
	}
}

static void drawButtonWidget(Widget *widget) {
	if (widget == app.activeWidget) {
		//if this is the active widget, draw it highlighted in white, with the selection indicator to its left
		drawTextDropShadow(widget->text, widget->x, widget->y, PALETTE_WHITE, widget->textAlignHorz, 0, PALETTE_BLACK, 1);
	} else {
		//draw all other widgets in blue
		drawTextDropShadow(widget->text, widget->x, widget->y, PALETTE_LIGHT_BLUE, widget->textAlignHorz, 0, PALETTE_BLACK, 1);
	}
}

static void drawSelectWidget(Widget *widget) {
	char          text[MAX_STRING_LENGTH];	//text buffer for options
	SelectWidget *selectWidget = (SelectWidget *)widget->data;	//fetch subwidget

	//draw parent widget's text
	if (widget == app.activeWidget) {
		//if this is the active widget, draw it highlighted in white, with the selection indicator to its left
		drawTextDropShadow(widget->text, widget->x, widget->y, PALETTE_WHITE, widget->textAlignHorz, 0, PALETTE_BLACK, 1);

		//draw subwidget with brackets around its text (maybe replace brackets with arrow sprites in the future?)
		sprintf(text, "%s", selectWidget->options[selectWidget->value]);
		drawTextDropShadow(text, selectWidget->x, selectWidget->y, PALETTE_WHITE, TAH_LEFT, 0, PALETTE_BLACK, 1);
	} else {
		//draw all other widgets in blue
		drawTextDropShadow(widget->text, widget->x, widget->y, PALETTE_LIGHT_BLUE, widget->textAlignHorz, 0, PALETTE_BLACK, 1);

		//draw subwidget with brackets around its text (maybe replace brackets with arrow sprites in the future?)
		sprintf(text, "%s", selectWidget->options[selectWidget->value]);
		drawTextDropShadow(text, selectWidget->x, selectWidget->y, PALETTE_LIGHT_BLUE, TAH_LEFT, 0, PALETTE_BLACK, 1);
	}
}

static void drawSliderWidget(Widget *widget) {
	SliderWidget *sliderWidget = (SliderWidget *)widget->data;	//fetch subwidget
	//find the normalized value of the slider bar
	float width = (float)(sliderWidget->value - sliderWidget->minValue) / (float)(sliderWidget->maxValue - sliderWidget->minValue);
	SDL_Rect sliderBarFiller = {sliderWidget->x + 4, sliderWidget->y + 4, 88 * width, 8};	//magic numbers!

	if (widget == app.activeWidget) {
		//if this is the active widget, draw it highlighted in white, with the selection indicator to its left
		drawTextDropShadow(widget->text, widget->x, widget->y, PALETTE_WHITE, widget->textAlignHorz, 0, PALETTE_BLACK, 1);

		//draw slider bar
		blitSpriteStatic(sliderBarWhite, sliderWidget->x, sliderWidget->y);
		SDL_SetRenderDrawBlendMode(app.renderer, SDL_BLENDMODE_NONE);
		SDL_SetRenderDrawColor(app.renderer, PALETTE_WHITE.r, PALETTE_WHITE.g, PALETTE_WHITE.b, 255);
		SDL_RenderFillRect(app.renderer, &sliderBarFiller);
	} else {
		//draw all other widgets in blue
		drawTextDropShadow(widget->text, widget->x, widget->y, PALETTE_LIGHT_BLUE, widget->textAlignHorz, 0, PALETTE_BLACK, 1);
	
		//draw slider bar
		blitSpriteStatic(sliderBarBlue, sliderWidget->x, sliderWidget->y);
		SDL_SetRenderDrawBlendMode(app.renderer, SDL_BLENDMODE_NONE);
		SDL_SetRenderDrawColor(app.renderer, PALETTE_LIGHT_BLUE.r, PALETTE_LIGHT_BLUE.g, PALETTE_LIGHT_BLUE.b, 255);
		SDL_RenderFillRect(app.renderer, &sliderBarFiller);
	}
}

static void drawTextInputWidget(Widget *widget) {
	TextInputWidget *textInputWidget = (TextInputWidget *)widget->data;	//fetch subwidget

	//draw widget, highlighting it if it's active
	if (widget == app.activeWidget) {
		//draw parent widget text
		drawTextDropShadow(widget->text, widget->x, widget->y, PALETTE_WHITE, widget->textAlignHorz, 0, PALETTE_BLACK, 1);
		//draw text input widget's text (currently, input text is always left-aligned)
		drawTextDropShadow(textInputWidget->text, textInputWidget->x, textInputWidget->y, PALETTE_WHITE, TAH_LEFT, 0, PALETTE_BLACK, 1);
	}
	else {
		//draw parent widget text
		drawTextDropShadow(widget->text, widget->x, widget->y, PALETTE_LIGHT_BLUE, widget->textAlignHorz, 0, PALETTE_BLACK, 1);
		//draw text input widget's text (currently, input text is always left-aligned)
		drawTextDropShadow(textInputWidget->text, textInputWidget->x, textInputWidget->y, PALETTE_LIGHT_BLUE, TAH_LEFT, 0, PALETTE_BLACK, 1);
	}

	//draw blinking text input pipe when the user can input text using this widget
	if (focusInputWidget && app.activeWidget == widget && cursorBlink % FPS < FPS * 0.5) {
		int textWidth;
		findTextDimensions(textInputWidget->text, &textWidth, NULL);

		//align pipe to edge of text
		drawTextDropShadow("|", textInputWidget->x + textWidth + CURSOR_BLINK_OFFSET, textInputWidget->y, PALETTE_WHITE, TAH_LEFT, NULL, PALETTE_BLACK, 1);
	}
}

//functions for loading and saving preferences

//loads preferences.json into app.preferences
void loadPreferences(void) {
	cJSON *root;	//json parsing var
	char *text;		//text buffer for JSON

	//get JSON as text
	text = readFile("./data/save/preferences.json");

	//file does not yet exist
	//load default values for sound and music into the app struct
	if (text == NULL) {
		printf("WARNING - data/save/preferences.json was not found.\n");
		app.preferences.fullscreen = true;
		app.preferences.soundVolume = 5;
		app.preferences.musicVolume = 5;
		return;
	}

	//translate JSON text into a cJSON object
	root = cJSON_Parse(text);

	//preferences are stored in a JSON object, referred to by "root"
	if (root != NULL) {
		app.preferences.fullscreen = cJSON_GetObjectItem(root, "fullscreen")->valueint;
		app.preferences.soundVolume = cJSON_GetObjectItem(root, "soundVolume")->valueint;
		app.preferences.musicVolume = cJSON_GetObjectItem(root, "musicVolume")->valueint;
	}

	//clean up cJSON object and text buffer
	cJSON_Delete(root);
	free(text);
}

//saves preferences to JSON
void savePreferences(void) {
	cJSON *obj;	//json parsing var

	//save new array to file

	//open/create file
	FILE *save = fopen("./data/save/preferences.json", "wb");

	//error checking
	if (save == NULL) {
		printf("ERROR - data/save/preferences.json could not be opened for writing.\n");
	} else {
		//create a cJSON object, populate it with the user's preferences, and save the object to file
		obj = cJSON_CreateObject();
		cJSON_AddNumberToObject(obj, "fullscreen", app.preferences.fullscreen);
		cJSON_AddNumberToObject(obj, "soundVolume", app.preferences.soundVolume);
		cJSON_AddNumberToObject(obj, "musicVolume", app.preferences.musicVolume);
		
		char *text = cJSON_Print(obj);
		fprintf(save, text);

		//cleanup
		fclose(save);
		free(text);	//cJSON_Print dynamically allocates
	}
}