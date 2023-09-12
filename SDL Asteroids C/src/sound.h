#ifndef SOUND_H
#define SOUND_H

/*
* A header for sound and music functions.
*/

//specific channels for sound.
typedef enum {
	SC_ANY = -1,
	SC_PLAYER,		//for things that happen to the player's ship
	SC_PLAYER_FIRE,	//for player gunfire
	SC_ENEMY,		//for things that happen to enemies
	SC_ENEMY_FIRE,	//for enemy gunfire
	SC_HITSOUNDS,
	SC_UI			//for UI beeps and boops
} SoundChannel;

//enums that refer to the game's sound effects.
//it'd be neat to make a system where I don't need to make an enum for every sound effect
//since this does not seem like it'd scale well to a large project
//i could probably do something similar to a sprite atlas, 
//where there's a stored key-value pair where the key is the name of the sound file
typedef enum {
	//gameplay sounds
	SFX_ENEMY_HIT,
	SFX_ENEMY_KILL,
	SFX_PLAYER_HIT,
	SFX_PLAYER_KILL,
	SFX_POWER_UP,
	SFX_SHOT_FIRE_1,
	SFX_SHOT_FIRE_2,
	SFX_SHOT_FIRE_3,
	SFX_SHOT_FIRE_4,
	SFX_SHOT_FIRE_5,
	//ui sounds
	SFX_ACCEPT,
	SFX_BIG_ACCEPT,
	SFX_CLICK,
	SFX_DECLINE,

	SFX_MAX
} Sound;

void initSounds(void);
void loadMusic(char *filename);
void playMusic(bool loop);
void setMusicVolume(int volume);
void playSound(int id, int channel, bool loop, int panning);
void playSoundIsolated(int id, int channel, bool loop, int panning);
void setSoundVolume(int volume);
void deleteSounds(void);

#endif