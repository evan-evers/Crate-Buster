#include "common.h"

#include "background.h"
#include "./json/cJSON.h"
#include "enemies.h"
#include "highscores.h"
#include "input.h"
#include "mainMenu.h"
#include "player.h"
#include "powerups.h"
#include "sound.h"
#include "stage.h"
#include "ui.h"
#include "utility.h"

extern App app;
extern InputManager input;
extern Player* player;
extern Stage stage;
extern int enemySpawnTimer;
extern bool focusInputWidget;

void initStage(void);
static void logic(void);
static void draw(void);
void deleteStage(void);

static void waResume(void);
static void waOptions(void);
static void waQuitToTitle(void);
static void waQuitToDesktop(void);
static void initPauseMenu(void);
static void initOptionsMenu(void);
static void initGameOverMenu(void);
static void initQuitCheckMenu(void);
static void initHighscoreMenu(void);

static const int START_STAGE = FPS * 2;	//start stage after brief period of letting the player observe where the crates are
static const int GO_TO_HIGHSCORES = FPS * 3;	//start stage after brief period of letting the player observe where the crates are
static int stageStartTimer = 0;		//timer that gives a pause before the stage begins
static int highscoreTimer = 0;		//timer that gives a pause between game over and going to the highscore screens
static bool drawQuitPromptText = false;	//bool for a hack to get prompt text for when the user wants to quit when on the pause menu

void initStage(void) {
	//initialize/reset stage
	memset(&stage, 0, sizeof(Stage));

	//set function pointers
	app.delegate.logic = logic;
	app.delegate.draw = draw;

	//initialize stage lists
	stage.bulletHead = stage.bulletTail = NULL;
	stage.crateHead = stage.crateTail = NULL;
	stage.enemyHead = stage.enemyTail = NULL;
	stage.particleHead = stage.particleTail = NULL;

	//initialize stage vars
	stage.timer = 0;
	stage.score = 0;
	stage.level = 1;
	stage.state = SS_BEGINNING;

	//initialize objects that must exist at start of level
	initHighscores();

	initPlayer(SCREEN_WIDTH * 0.5, SCREEN_HEIGHT * 0.5);

	initCrates(1 + stage.level * 2);

	initPowerups();

	initGameplayUI();

	//init stage start timer so first stage doesn't instantly begin
	stageStartTimer = 0;

	//set cursor
	app.cursorState = CS_RETICLE;

	//stop music; gameplay music will be started when after get ready screen
	loadMusic(NULL);
}

static void resetStage() {
	//delete any remaining bullets, offscreen enemies, and particles
	deleteBullets();

	deleteEnemies();

	deleteCrates();

	//get rid of any remaining scrap
	Particle *particle = stage.particleHead;

	while (particle != NULL) {
		//using the location of the power and scrap particles' sprites on their sprite sheet to identify them
		//this is a bit hacky but oh well, i'm making a real sprite atlas after this project anyway
		if (particle->sprite != NULL) {
			//delete scrap
			if (particle->sprite->srcY >= 17 * SPRITE_ATLAS_CELL_H && particle->sprite->srcY <= 20 * SPRITE_ATLAS_CELL_H) {
				particle->ttl = 0;
			}
			//delete powerups
			if (particle->sprite->srcX >= 0 * SPRITE_ATLAS_CELL_W && particle->sprite->srcX <= 7 * SPRITE_ATLAS_CELL_W && particle->sprite->srcY == 14 * SPRITE_ATLAS_CELL_H) {
				particle->ttl = 0;
			}
		}

		particle = particle->next;
	}

	//reset stage vars
	++stage.level;
	stage.state = SS_BEGINNING;

	//reset objects that must exist at start of level
	player->x = SCREEN_WIDTH * 0.5;
	player->y = SCREEN_HEIGHT * 0.5;
	player->momentumVector.x = 0;
	player->momentumVector.y = 0;

	//add crates to the stage (increase with stage level)
	initCrates(1 + stage.level * 2);

	//reset stage start timer so stage doesn't instantly begin
	stageStartTimer = 0;

	//set cursor
	app.cursorState = CS_RETICLE;
}

static void ssBeginningUpdate(void) {
	//increment timer
	++stageStartTimer;

	//ensures that the scrap from the last level is deleted via ttl system
	updateParticles();

	//state change

	//when timer hits a certain point, start the stage
	if (stageStartTimer >= START_STAGE) {
		//on first stage, wait 20 seconds to spawn first enemy; decrement by 2 seconds for each new stage level
		enemySpawnTimer = MAX(FPS * 20 - (stage.level - 1) * FPS * 2, FPS * 5);

		//start music if on first stage
		if (stage.level == 1) {
			loadMusic("sound/music/Asteroids_Clone_Gameplay_Theme.wav");
			playMusic(true);
		}

		stage.state = SS_GAMEPLAY;
	}
}

static void ssGameplayUpdate(void) {
	updatePlayer();

	updateParticles();

	updateCrates();

	updateBullets();

	updateEnemies();

	spawnEnemies();

	//increment timer
	++stage.timer;

	//state change

	//if player is dead, go to game over state
	if (player->state == PS_DESTROYED) {
		highscoreTimer = 0;

		//stop music
		loadMusic(NULL);

		stage.state = SS_GAME_OVER;
	}

	//when all crates are destroyed and all enemies are either destroyed or offscreen, end stage
	if (stage.crateHead == NULL) {
		if (stage.enemyHead == NULL) {
			//play success sound
			playSound(SFX_BIG_ACCEPT, SC_UI, false, PAN_CENTER);

			//delete any remaining enemy bullets
			Bullet *bullet = stage.bulletHead;

			while (bullet != NULL) {
				if (bullet->type == BT_ENEMY)
					bullet->ttl = 0;

				bullet = bullet->next;
			}

			stage.state = SS_END;
		}
		else {
			//check for an onscreen enemy
			//if no such enemy exists, change states
			Enemy *enemy = stage.enemyHead;
			bool enemyOnscreen = false;
			float horzEdgeDist = enemy->sprite->w * SCREENWRAP_MARGIN + 1;
			float vertEdgeDist = enemy->sprite->h * SCREENWRAP_MARGIN + 1;

			while (enemy != NULL) {
				if (enemy->x > -horzEdgeDist && enemy->x < SCREEN_WIDTH + horzEdgeDist && enemy->y > -vertEdgeDist && enemy->y < SCREEN_HEIGHT + vertEdgeDist) {
					enemyOnscreen = true;
					break;
				}

				enemy = enemy->next;
			}

			if (!enemyOnscreen) {
				//play success sound
				playSound(SFX_BIG_ACCEPT, SC_UI, false, PAN_CENTER);

				//delete any remaining enemy bullets
				Bullet *bullet = stage.bulletHead;

				while (bullet != NULL) {
					if (bullet->type == BT_ENEMY)
						bullet->ttl = 0;

					bullet = bullet->next;
				}

				stage.state = SS_END;
			}

			enemy = NULL;
		}
	}
	
	//pause if the player hits the pause button
	if (input.pausePressed > 0) {
		input.pausePressed = 0;

		initPauseMenu();

		stage.state = SS_PAUSED;
	}
}

static void ssEndUpdate(void) {
	//update certain things so that the screen isn't stuck in a weird freeze and so that the player can collect any powerups left on screen
	updatePlayer();

	updateParticles();

	updateBullets();

	//state change

	//when player presses fire, move on to the next stage
	if (input.firePressed > 0) {
		input.firePressed = 0;

		//reset stage has the state change built in
		resetStage();
	}
}

static void ssGameOverUpdate(void) {
	//keep updating things in the level so that there isn't an awkward freeze
	updatePlayer();

	updateParticles();

	updateCrates();

	updateBullets();

	updateEnemies();

	//state change
	++highscoreTimer;

	//update game over menu (state change is handled by the menu's action functions)
	if (highscoreTimer >= GO_TO_HIGHSCORES) {
		//don't delete stage here; let the widgets do it
		//this does mean that the stage will be running, undrawn, while the highscore table is being viewed
		//in a non-CPU intensive game like this, though, that's fine.

		//set cursor
		app.cursorState = CS_POINTER;

		app.latestHighscoreIndex = -1;	//default to not highlighting a highscore table score

		//fetch highscores both loads the savefile's highscores into the local highscores array,
		//and checks if the player's score is high enough to get on the highscores table
		if (loadHighscores()) {
			stage.state = SS_INPUT_HIGHSCORE;
			initHighscoreMenu();
		}
		else {
			stage.state = SS_HIGHSCORE_TABLE;
			initGameOverMenu();
		}
	}
}

static void ssInputHighscoreUpdate() {
	//make sure the input widget is always being focused on
	focusInputWidget = true;

	//state change built into widget actions
	updateWidgets(NULL);
}

static void ssHighscoreTableUpdate() {
	//state change built into widget actions
	updateWidgets(NULL);
}

static void ssPausedUpdate(void) {
	//update none of the gameplay elements

	//update pause menu
	updateWidgets(NULL);

	//state change
	
	//unpause if the player hits the pause button
	if (input.pausePressed > 0) {
		input.pausePressed = 0;

		//set cursor
		app.cursorState = CS_RETICLE;

		//save user preferences in case the user exits the options menu using this
		//if i'd used a state machine here i could have just restricted unpausing to only happen on the main pause menu
		//lesson learned
		savePreferences();

		//unload widgets
		deleteWidgets();

		//change state
		stage.state = SS_GAMEPLAY;
	}
}

static void logic(void) {
	switch (stage.state) {
		case(SS_BEGINNING):
			ssBeginningUpdate();
			break;
		case(SS_GAMEPLAY):
			ssGameplayUpdate();
			break;
		case(SS_END):
			ssEndUpdate();
			break;
		case(SS_GAME_OVER):
			ssGameOverUpdate();
			break;
		case(SS_INPUT_HIGHSCORE):
			ssInputHighscoreUpdate();
			break;
		case(SS_HIGHSCORE_TABLE):
			ssHighscoreTableUpdate();
			break;
		case(SS_PAUSED):
			ssPausedUpdate();
			break;
	}
}

static void ssBeginningDraw() {
	drawBackground();

	drawParticles();

	drawCrates();

	drawEnemies();

	drawBullets();

	drawPlayer();

	drawStageStartUI();
}

static void ssGameplayDraw() {
	drawBackground();

	drawParticles();

	drawCrates();

	drawEnemies();

	drawBullets();

	drawPlayer();

	drawGameplayUI();
}

static void ssEndDraw() {
	drawBackground();

	drawParticles();

	drawCrates();

	drawEnemies();

	drawBullets();

	drawPlayer();

	drawStageEndUI();
}

static void ssGameOverDraw() {
	drawBackground();

	drawParticles();

	drawCrates();

	drawEnemies();

	drawBullets();

	drawPlayer();

	drawStageGameOverUI();
}

static void ssInputHighscoreDraw() {
	drawBackground();

	//draw prompt text
	drawTextDropShadow("Congratulations! You just got a top score.", SCREEN_WIDTH * 0.5, SCREEN_HEIGHT * 0.5 - 30, PALETTE_LIGHT_BLUE, TAH_CENTER, NULL, PALETTE_BLACK, 1);
	drawTextDropShadow("Enter your name below:", SCREEN_WIDTH * 0.5, SCREEN_HEIGHT * 0.5, PALETTE_LIGHT_BLUE, TAH_CENTER, NULL, PALETTE_BLACK, 1);

	//draw widgets
	drawWidgets(NULL);
}

static void ssHighscoreTableDraw() {
	drawBackground();

	drawHighscores(true);

	//draw widgets
	drawWidgets(NULL);
}

static void ssPausedDraw() {
	//using player to check if everything's been deleted yet (in case of quitting to title)
	if (player != NULL) {
		drawBackground();

		drawParticles();

		drawCrates();

		drawEnemies();

		drawBullets();

		drawPlayer();

		//don't draw gameplay UI; it's a bit confusing to look at with the menu on screen

		drawPausedUI();

		//here's a hack for prompt text, caused by the way I implemented widgets for the pause screen
		if (drawQuitPromptText) {
			drawTextDropShadow("Are you sure you want to quit?", SCREEN_WIDTH * 0.5, 135, PALETTE_LIGHT_BLUE, TAH_CENTER, NULL, PALETTE_BLACK, 1);
			drawTextDropShadow("Your progress will be lost.", SCREEN_WIDTH * 0.5, 165, PALETTE_LIGHT_BLUE, TAH_CENTER, NULL, PALETTE_BLACK, 1);
		}
	}
}

static void draw(void) {
	switch (stage.state) {
	case(SS_BEGINNING):
		ssBeginningDraw();
		break;
	case(SS_GAMEPLAY):
		ssGameplayDraw();
		break;
	case(SS_END):
		ssEndDraw();
		break;
	case(SS_GAME_OVER):
		ssGameOverDraw();
		break;
	case(SS_INPUT_HIGHSCORE):
		ssInputHighscoreDraw();
		break;
	case(SS_HIGHSCORE_TABLE):
		ssHighscoreTableDraw();
		break;
	case(SS_PAUSED):
		ssPausedDraw();
		break;
	}
}

void deleteStage(void) {
	deleteGameplayUI();

	deleteParticles();

	deleteCrates();

	deleteBullets();

	deletePlayer();

	deleteEnemies();

	deletePowerups();

	deleteWidgets();
}



//widget action functions

//quit check menu functions

static void waQuitCheckYesTitle(void) {
	//unload stage (deletes this menu's widgets, too)
	deleteStage();

	//load title
	initMainMenu();

	//stop showing the quit prompt (hack)
	drawQuitPromptText = false;
}

static void waQuitCheckYesDesktop(void) {
	//unload stage (deletes this menu's widgets, too)
	deleteStage();

	//quit game
	app.quit = true;

	//stop showing the quit prompt (hack)
	drawQuitPromptText = false;
}

//pause menu functions

static void waResume(void) {
	//unload widgets
	deleteWidgets();

	//set cursor
	app.cursorState = CS_RETICLE;

	//change state
	stage.state = SS_GAMEPLAY;
}

static void waOptions(void) {
	//unload pause menu widgets
	deleteWidgets();

	//load options menu widgets
	initOptionsMenu();
}

static void waQuitToTitle(void) {
	//unload pause menu widgets
	deleteWidgets();

	//load quit check widgets
	initQuitCheckMenu();

	//set the action of the quit check widget
	Widget *widget = getWidget("yes", "quitCheck");
	widget->action = waQuitCheckYesTitle;

	//hack to get quit text prompt working
	drawQuitPromptText = true;
}

static void waQuitToDesktop(void) {
	//unload pause menu widgets
	deleteWidgets();

	//load quit check widgets
	initQuitCheckMenu();

	//set the action of the quit check widget
	Widget *widget = getWidget("yes", "quitCheck");
	widget->action = waQuitCheckYesDesktop;

	//hack to get quit text prompt working
	drawQuitPromptText = true;
}

//options menu functions

//fullscreenToggle, SFXSlider and MusicSlider aren't static so that mainMenu.c can use them & so I don't have to duplicate them for the main menu options menu
void waFullscreenToggle(void) {
	if (app.preferences.fullscreen) {
		SDL_SetWindowFullscreen(app.window, 0);	//go windowed
		app.preferences.fullscreen = false;
	}
	else {
		SDL_SetWindowFullscreen(app.window, SDL_WINDOW_FULLSCREEN_DESKTOP);	//go fullscreen
		app.preferences.fullscreen = true;
	}

	//the "back" widget saves these preferences
}

void waSFXSlider(void) {
	//get a pointer to the sfx slider widget
	//seems a bit inefficient to have to get a pointer to this widget when the widget we want is the same widget that's calling this function
	SliderWidget *sliderWidget = (SliderWidget *)getWidget("sfxSlider", "options")->data;

	//put value in app struct
	app.preferences.soundVolume = sliderWidget->value;

	//set the actual volume
	setSoundVolume(app.preferences.soundVolume);

	//the "back" widget saves these preferences
}

void waMusicSlider(void) {
	//get a pointer to the sfx slider widget
	//seems a bit inefficient to have to get a pointer to this widget when the widget we want is the same widget that's calling this function
	SliderWidget *sliderWidget = (SliderWidget *)getWidget("musicSlider", "options")->data;

	//put value in app struct
	app.preferences.musicVolume = sliderWidget->value;

	//set the actual volume
	setMusicVolume(app.preferences.musicVolume);

	//the "back" widget saves these preferences
}

//goes from the options menu back to the pause menu
static void waBack(void) {
	//unload options menu widgets
	deleteWidgets();

	//save user preferences
	savePreferences();

	//load pause menu widgets
	initPauseMenu();

	//if this was called from a quit prompt menu, stop showing the quit prompt
	drawQuitPromptText = false;
}

static void waTryAgain(void) {
	//unload options widgets
	deleteWidgets();

	//reset level and score
	stage.level = 0;
	stage.score = 0;

	//reset player
	resetPlayer(SCREEN_WIDTH * 0.5, SCREEN_HEIGHT * 0.5);

	//reset stage has the state change built in
	resetStage();
}

static void waInputName(void) {
	//save highscores while we still have widget text info
	saveHighscores();

	//delete widgets
	deleteWidgets();

	//load game over menu widgets
	initGameOverMenu();

	//state change
	stage.state = SS_HIGHSCORE_TABLE;
}

//initializes only the pause menu during the pause state
static void initPauseMenu(void) {
	//initialize pause menu widgets
	loadWidgets("data/widgets/paused.json");

	//set cursor
	app.cursorState = CS_POINTER;

	Widget *widget;

	widget = getWidget("resume", "paused");
	widget->action = waResume;
	widget->x = SCREEN_WIDTH * 0.5;
	app.activeWidget = widget;	//set resume button as the active widget

	widget = getWidget("options", "paused");
	widget->action = waOptions;
	widget->x = SCREEN_WIDTH * 0.5;

	widget = getWidget("quitToTitle", "paused");
	widget->action = waQuitToTitle;
	widget->x = SCREEN_WIDTH * 0.5;

	widget = getWidget("quitToDesktop", "paused");
	widget->action = waQuitToDesktop;
	widget->x = SCREEN_WIDTH * 0.5;
}

//initializes only the options menu during the pause state
static void initOptionsMenu(void) {
	//initialize options menu widgets
	loadWidgets("data/widgets/options.json");

	Widget *widget;

	widget = getWidget("displayMode", "options");
	widget->action = waFullscreenToggle;
	widget->x = SCREEN_WIDTH * 0.5;
	app.activeWidget = widget;	//set this as the active widget
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

//initializes the quit check menu
//I now see the point of using widget groups; while it might be slightly more memory efficient to load and unload widgets every time a menu changes, it's a real pain to program
//premature optimization is a cardinal sin
static void initQuitCheckMenu(void) {
	//initialize options menu widgets
	loadWidgets("data/widgets/quitCheck.json");

	Widget *widget;

	widget = getWidget("yes", "quitCheck");
	//action isn't initialized here, but rather in action that initializes this menu
	//this is done because this widget needs to call a different action depending on whether the player is quitting to title or quitting to desktop
	widget->x = SCREEN_WIDTH * 0.5;

	widget = getWidget("no", "quitCheck");
	widget->action = waBack;
	widget->x = SCREEN_WIDTH * 0.5;

	app.activeWidget = widget;	//set no as the active widget, to prevent an accidental double-tap from quitting the program
}

//initializes game over menu
//this is now called on displaying th4e highscore table
static void initGameOverMenu(void) {
	//initialize pause menu widgets
	loadWidgets("data/widgets/gameOver.json");

	Widget *widget;

	widget = getWidget("tryAgain", "gameOver");
	widget->action = waTryAgain;
	widget->x = SCREEN_WIDTH * 0.5;
	app.activeWidget = widget;	//set this as the active widget

	widget = getWidget("quitToTitle", "gameOver");
	widget->action = waQuitCheckYesTitle;
	widget->x = SCREEN_WIDTH * 0.5;

	widget = getWidget("quitToDesktop", "gameOver");
	widget->action = waQuitCheckYesDesktop;
	widget->x = SCREEN_WIDTH * 0.5;
}

static void initHighscoreMenu(void) {
	//reset text buffer
	input.inputText[0] = '\0';

	//initialize pause menu widgets
	loadWidgets("data/widgets/highscoreInput.json");

	Widget *widget;

	widget = getWidget("inputName", "highscoreInput");
	widget->action = waInputName;
	widget->x = SCREEN_WIDTH * 0.5;
	TextInputWidget *textInputWidget = (TextInputWidget *)widget->data;
	textInputWidget->x = SCREEN_WIDTH * 0.5;
	textInputWidget->y = widget->y;
	textInputWidget->maxLength = MAX_NAME_LENGTH;
	app.activeWidget = widget;	//set this as the active widget
}