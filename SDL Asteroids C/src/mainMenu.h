#ifndef MAIN_MENU_H
#define MAIN_MENU_H

/*
* Header file for the main menu
*/

#include "common.h"

//enum for main menu's state machine
typedef enum {
	MMS_MAIN_MENU,
	MMS_HOW_TO_PLAY,
	MMS_HIGHSCORES,
	MMS_OPTIONS,
	MMS_CREDITS
	//quit doesn't have an enum b/c the quit state is just closing the game app.
} MainMenuState;

void initMainMenu(void);
void deleteMainMenu(void);

#endif