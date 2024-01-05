#include "common.h"

#include "background.h"
#include "cursor.h"
#include "enemies.h"
#include "highscores.h"
#include "input.h"
#include "mainMenu.h"
#include "player.h"
#include "sound.h"
#include "stage.h"
#include "ui.h"
#include "widgets.h"

extern App app;
extern InputManager input;
extern Player *player;

static void logic(void);
static void draw(void);
static void initMainMenuWidgets(void);
static MainMenuState mmState;

void initMainMenu(void) {
	//set function pointers
	app.delegate.logic = logic;
	app.delegate.draw = draw;

	//initialize stuff
	mmState = MMS_MAIN_MENU;	//start on main menu

	//set cursor
	app.cursorState = CS_POINTER;

	initMainMenuUI();

	initMainMenuWidgets();

	initHighscores();

	//start music
	loadMusic("sound/music/AC_Music_Menu_Theme.ogg");
	playMusic(true);
}

static void logic(void) {
	switch (mmState) {
	case(MMS_MAIN_MENU):
		updateWidgets("title");
		break;
	case(MMS_HOW_TO_PLAY):
		updateWidgets("back");
		break;
	case(MMS_HIGHSCORES):
		updateWidgets("back");
		break;
	case(MMS_OPTIONS):
		updateWidgets("options");
		break;
	case(MMS_CREDITS):
		updateWidgets("back");
		break;
	}
}

static void draw(void) {
	switch (mmState) {
	case(MMS_MAIN_MENU):
		drawMainMenuUI();
		break;
	case(MMS_HOW_TO_PLAY):
		drawHowToPlayUI();
		break;
	case(MMS_HIGHSCORES):
		drawHighscoresUI();
		break;
	case(MMS_OPTIONS):
		drawOptionsUI();
		break;
	case(MMS_CREDITS):
		drawCreditsUI();
		break;
	}
}

void deleteMainMenu(void) {
	deleteWidgets();

	deleteMainMenuUI();
}


//widget functions section

static void waPlay(void) {
	//unload main menu
	deleteMainMenu();

	//play an exiting sound
	playSound(SFX_BIG_ACCEPT, SC_UI, false, PAN_CENTER);

	//start stage
	initStage();
}

static void waHowToPlay(void) {
	//change main menu state
	mmState = MMS_HOW_TO_PLAY;

	//set active widget
	app.activeWidget = getWidget("back", "back");
}

static void waHighscores(void) {
	//change main menu state
	mmState = MMS_HIGHSCORES;

	//bring up saved highscores, if any
	loadHighscores();

	//set active widget
	app.activeWidget = getWidget("back", "back");
}

static void waOptions(void) {
	//change main menu state
	mmState = MMS_OPTIONS;

	app.activeWidget = getWidget("displayMode", "options");
}

static void waCredits(void) {
	//change main menu state
	mmState = MMS_CREDITS;

	//set active widget
	app.activeWidget = getWidget("back", "back");
}

static void waQuitToDesktop(void) {
	//unload main menu
	deleteMainMenu();

	//quit game
	app.quit = true;
}

static void waBack(void) {
	//if exiting options menu, save user preferences
	if (mmState == MMS_OPTIONS)
		savePreferences();

	//go back to title screen
	mmState = MMS_MAIN_MENU;

	//set active widget
	app.activeWidget = getWidget("play", "title");
}

//initializes main menu's widgets
static void initMainMenuWidgets(void) {
	//title.json holds the main menu widgets and the back button
	loadWidgets("data/widgets/title.json");

	Widget *widget;

	widget = getWidget("play", "title");
	widget->action = waPlay;
	widget->x = SCREEN_WIDTH * 0.5;
	app.activeWidget = widget;	//set play button as the active widget

	widget = getWidget("howToPlay", "title");
	widget->action = waHowToPlay;
	widget->x = SCREEN_WIDTH * 0.5;

	widget = getWidget("highscores", "title");
	widget->action = waHighscores;
	widget->x = SCREEN_WIDTH * 0.5;

	widget = getWidget("options", "title");
	widget->action = waOptions;
	widget->x = SCREEN_WIDTH * 0.5;

	widget = getWidget("credits", "title");
	widget->action = waCredits;
	widget->x = SCREEN_WIDTH * 0.5;

	widget = getWidget("quitToDesktop", "title");
	widget->action = waQuitToDesktop;
	widget->x = SCREEN_WIDTH * 0.5;

	//generic back button for how to play & credits
	widget = getWidget("back", "back");
	widget->action = waBack;
	widget->x = SCREEN_WIDTH * 0.5;

	//initialize options menu widgets
	loadWidgets("data/widgets/options.json");

	widget = getWidget("displayMode", "options");
	widget->action = waFullscreenToggle;
	widget->x = SCREEN_WIDTH * 0.5;
	SelectWidget *selectWidget = (SelectWidget *)widget->data;
	selectWidget->x = SCREEN_WIDTH * 0.5;
	selectWidget->y = widget->y;
	selectWidget->value = (int)app.preferences.fullscreen;	//check the app struct for the current fullscreen state

	widget = getWidget("sfxSlider", "options");
	widget->action = waSFXSlider;
	widget->x = SCREEN_WIDTH * 0.5;
	SliderWidget *sliderWidget = (SliderWidget *)widget->data;
	sliderWidget->x = SCREEN_WIDTH * 0.5;
	sliderWidget->y = widget->y;
	sliderWidget->value = app.preferences.soundVolume;

	widget = getWidget("musicSlider", "options");
	widget->action = waMusicSlider;
	widget->x = SCREEN_WIDTH * 0.5;
	sliderWidget = (SliderWidget *)widget->data;
	sliderWidget->x = SCREEN_WIDTH * 0.5;
	sliderWidget->y = widget->y;
	sliderWidget->value = app.preferences.musicVolume;

	widget = getWidget("back", "options");
	widget->action = waBack;
	widget->x = SCREEN_WIDTH * 0.5;
}