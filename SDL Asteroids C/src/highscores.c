#include "common.h"

#include "json/cJSON.h"
#include "highscores.h"
#include "stage.h"
#include "utility.h"

extern App app;
extern Stage stage;

static Highscore highscores[NUM_HIGHSCORES];	//holds highscores when they've been loaded from the highscores.json file
static const int HIGHSCORE_TEXT_OFFSET_HORZ = 150;	//aesthetic positional vars
static const int HIGHSCORE_TEXT_OFFSET_VERT = 60;
static char highscoreTextBuffer[32];	//if the player gets a score with more than 31 digits I'll eat my socks.

void initHighscores(void) {
	//initialize highscores array with negative scores
	//this way we can check if there's still blank spaces on the highscore table
	for (int i = 0; i < NUM_HIGHSCORES; ++i) {
		highscores[i].score = -1;
	}
}

//loads highscores.json into local array "highscores"
//returns true if the player's score is high enough to get on the table,
//and false if the player's score doesn't make it
bool loadHighscores(void) {
	cJSON *root, *node;	//json parsing variables
	char *text;			//text buffer for JSON

	//get JSON as text
	text = readFile("./data/save/highscores.json");

	//file does not yet exist
	//since there's no highscore data yet, the player definitely makes it onto the board
	//file will be created in saveHighscores
	if (text == NULL) {
		printf("WARNING - data/save/highscores.json was not found.\n");
		return true;
	}

	//translate JSON text into a cJSON object
	root = cJSON_Parse(text);

	//highscores are stored as a cJSON array; iterate through that array
	//"root" is a cJSON array of cJSON objects (the highscore objects themselves)
	//a cJSON array is accessed as a linked list pointed to by (array name)->child (in this case, root->child)
	int i = 0;	//iterates through the highscores array

	if (root != NULL && root->child != NULL) {
		for (node = root->child; node != NULL; node = node->next) {
			//copy in any information currently in the highscores savefile
			STRNCPY(highscores[i].name, cJSON_GetObjectItem(node, "name")->valuestring, MAX_NAME_LENGTH);
			highscores[i].score = cJSON_GetObjectItem(node, "score")->valueint;

			++i;
		}
	}

	//clean up cJSON object and text buffer
	cJSON_Delete(root);
	free(text);

	//if there are less than NUM_HIGHSCORES highscores saved right now, we want this score put on the highscores table
	//if the player's score is greater than the smallest highscore, we want this score put on the highscores table
	//otherwise, go straight to highscore table and do not save this score
	//check the smallest highscore and see
	if (i < NUM_HIGHSCORES || stage.score > highscores[NUM_HIGHSCORES - 1].score)
		return true;
	else
		return false;
}

//places one of the user's highscores in sorted position in the "highscores" array before saving it
//returns the position in the array where the user's new highscore is
int saveHighscores(void) {
	cJSON *array, *item;	//json parsing variables
	TextInputWidget *textInputWidget = (TextInputWidget *)getWidget("inputName", "highscoreInput")->data;	//get access to the text the user just entered
	int i;	//index of user's score in the highscores array

	//find where the user's score belongs in the array
	for (i = 0; i < NUM_HIGHSCORES; ++i) {
		if (stage.score > highscores[i].score)
			break;
	}

	//shift scores in the highscores array to accommodate for the new highscore
	for (int j = NUM_HIGHSCORES - 1; j > i; --j) {
		STRNCPY(highscores[j].name, highscores[j - 1].name, MAX_NAME_LENGTH + 1);
		highscores[j].score = highscores[j - 1].score;
	}

	//add new highscore to the highscores array
	STRNCPY(highscores[i].name, textInputWidget->text, MAX_NAME_LENGTH + 1);
	highscores[i].score = stage.score;

	//save new array to file

	//open file
	FILE *save = fopen("./data/save/highscores.json", "wb");

	//error checking
	if (save == NULL) {
		printf("ERROR - data/save/highscores.json could not be opened for writing.\n");
	} else {
		//create an array of highscore objects in cJSON, convert that to JSON text, and print that text to a file
		array = cJSON_CreateArray();
		for (int j = 0; j < NUM_HIGHSCORES; ++j) {
			//make sure we only write highscores that exist
			if (highscores[j].score == -1)
				break;

			item = cJSON_CreateObject();
			cJSON_AddStringToObject(item, "name", highscores[j].name);
			cJSON_AddNumberToObject(item, "score", highscores[j].score);
			cJSON_AddItemToArray(array, item);
		}
		char *text = cJSON_Print(array);
		fprintf(save, text);

		//cleanup
		fclose(save);
		free(text);	//cJSON_Print dynamically allocates
	}

	return i;	//position in array where user's highscore is
}

//draws the highscore table (does not include widgets to navigate away from it)
//pass in true to include the player's latest score in the layout, regardless of if they got on the highscore table or not
void drawHighscores(bool includeLatestScore) {
	//draw header text
	drawTextDropShadow("Top 5:", SCREEN_WIDTH * 0.5, 30, PALETTE_LIGHT_BLUE, TAH_CENTER, NULL, PALETTE_BLACK, 1);

	//draw highscores
	for (int i = 0; i < NUM_HIGHSCORES; ++i) {
		//check if the score's real
		if (highscores[i].score > -1) {
			if (i == app.latestHighscoreIndex && includeLatestScore) {
				//display latest highscore highlighted
				drawTextDropShadow(highscores[i].name, SCREEN_WIDTH * 0.5 - HIGHSCORE_TEXT_OFFSET_HORZ, HIGHSCORE_TEXT_OFFSET_VERT + 30 * i, PALETTE_WHITE, TAH_LEFT, NULL, PALETTE_BLACK, 1);
				sprintf(highscoreTextBuffer, "%d", highscores[i].score);
				drawTextDropShadow(highscoreTextBuffer, SCREEN_WIDTH * 0.5 + HIGHSCORE_TEXT_OFFSET_HORZ, HIGHSCORE_TEXT_OFFSET_VERT + 30 * i, PALETTE_WHITE, TAH_RIGHT, NULL, PALETTE_BLACK, 1);
			} else {
				//display normally
				drawTextDropShadow(highscores[i].name, SCREEN_WIDTH * 0.5 - HIGHSCORE_TEXT_OFFSET_HORZ, HIGHSCORE_TEXT_OFFSET_VERT + 30 * i, PALETTE_LIGHT_BLUE, TAH_LEFT, NULL, PALETTE_BLACK, 1);
				sprintf(highscoreTextBuffer, "%d", highscores[i].score);
				drawTextDropShadow(highscoreTextBuffer, SCREEN_WIDTH * 0.5 + HIGHSCORE_TEXT_OFFSET_HORZ, HIGHSCORE_TEXT_OFFSET_VERT + 30 * i, PALETTE_LIGHT_BLUE, TAH_RIGHT, NULL, PALETTE_BLACK, 1);
			}
		} else {
			//display dashes
			drawTextDropShadow("-", SCREEN_WIDTH * 0.5 - HIGHSCORE_TEXT_OFFSET_HORZ, HIGHSCORE_TEXT_OFFSET_VERT + 30 * i, PALETTE_LIGHT_BLUE, TAH_LEFT, NULL, PALETTE_BLACK, 1);
			drawTextDropShadow("-", SCREEN_WIDTH * 0.5 + HIGHSCORE_TEXT_OFFSET_HORZ, HIGHSCORE_TEXT_OFFSET_VERT + 30 * i, PALETTE_LIGHT_BLUE, TAH_RIGHT, NULL, PALETTE_BLACK, 1);
		}
	}

	//draw player's score that round if it's desired by the function caller
	if (includeLatestScore) {
		sprintf(highscoreTextBuffer, "Your score: %d", stage.score);
		drawTextDropShadow(highscoreTextBuffer, SCREEN_WIDTH * 0.5, 210, PALETTE_WHITE, TAH_CENTER, NULL, PALETTE_BLACK, 1);
	}
}