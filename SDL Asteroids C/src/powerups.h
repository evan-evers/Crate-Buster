#ifndef POWERUPS_H
#define POWERUPS_H

/*
* Header file for powerups.
* Powerups are implemented as a type of particle, so no unique struct needed.
*/

void initPowerup(int x, int y);
void initPowerups(void);
void deletePowerups(void);

#endif