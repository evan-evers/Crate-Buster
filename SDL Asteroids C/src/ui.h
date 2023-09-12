#ifndef UI_H
#define UI_H

/*
* Header file for gameplay and menu UI.
*/

void initGameplayUI(void);
void drawStageStartUI(void);
void drawGameplayUI(void);
void drawStageEndUI(void);
void drawStageGameOverUI(void);
void drawPausedUI(void);
void deleteGameplayUI(void);

void initMainMenuUI(void);
void drawMainMenuUI(void);
void drawHowToPlayUI(void);
void drawHighscoresUI(void);
void drawOptionsUI(void);
void drawCreditsUI(void);
void deleteMainMenuUI(void);

#endif