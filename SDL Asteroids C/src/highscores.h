#ifndef HIGHSCORES_H
#define HIGHSCORES_H

/*
* Functions and structs related to the highscores table.
*/

//holds a highscore and information related to it
typedef struct {
	char name[MAX_NAME_LENGTH + 1];
	int score;
} Highscore;

void initHighscores(void);
bool loadHighscores(void);
int saveHighscores(void);
void drawHighscores(bool includeLatestScore);

#endif