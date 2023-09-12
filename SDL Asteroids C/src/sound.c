#include "common.h"

#include "SDL_mixer.h"

#include "sound.h"

extern App app;

static void loadSounds(void);

//constants for adjusting the volumes and music and sound relative to each other
//a very primitive form of mixing
static const float SOUND_VOL_ADJUST = 1.0;
static const float MUSIC_VOL_ADJUST = 0.4;

//array of pointers to all sounds the game will use
//this is another element of this sound system that wouldn't work well for a large project
//since every sound would be loaded at once
static Mix_Chunk *sounds[SFX_MAX];
static Mix_Music *music;			//pointer to the currently loaded music track

//initializes and loads sounds
void initSounds(void) {
	//NULL the sound pointers
	memset(sounds, 0, sizeof(Mix_Chunk*) * SFX_MAX);

	//NULL the music pointer
	music = NULL;

	//load sounds into the array
	loadSounds();

	//set volume of sounds according to user preferences
	setSoundVolume(app.preferences.soundVolume);
}

//put all this tedious code in a function so i don't have to look at it
static void loadSounds(void) {
	//gameplay sounds
	sounds[SFX_ENEMY_HIT] = Mix_LoadWAV("sound/gameplay/SFX_Enemy_Hit.wav");
	sounds[SFX_ENEMY_KILL] = Mix_LoadWAV("sound/gameplay/SFX_Kill_Enemy.wav");
	sounds[SFX_PLAYER_HIT] = Mix_LoadWAV("sound/gameplay/SFX_Player_Hit.wav");
	sounds[SFX_PLAYER_KILL] = Mix_LoadWAV("sound/gameplay/SFX_Kill_Player.wav");
	sounds[SFX_POWER_UP] = Mix_LoadWAV("sound/gameplay/SFX_Power_Up.wav");
	sounds[SFX_SHOT_FIRE_1] = Mix_LoadWAV("sound/gameplay/SFX_Shot_Fire_1.wav");
	sounds[SFX_SHOT_FIRE_2] = Mix_LoadWAV("sound/gameplay/SFX_Shot_Fire_2.wav");
	sounds[SFX_SHOT_FIRE_3] = Mix_LoadWAV("sound/gameplay/SFX_Shot_Fire_3.wav");
	sounds[SFX_SHOT_FIRE_4] = Mix_LoadWAV("sound/gameplay/SFX_Shot_Fire_4.wav");
	sounds[SFX_SHOT_FIRE_5] = Mix_LoadWAV("sound/gameplay/SFX_Shot_Fire_5.wav");
	//ui sounds
	sounds[SFX_ACCEPT] = Mix_LoadWAV("sound/ui/SFX_Accept.wav");
	sounds[SFX_BIG_ACCEPT] = Mix_LoadWAV("sound/ui/SFX_Big_Accept.wav");
	sounds[SFX_CLICK] = Mix_LoadWAV("sound/ui/SFX_Click.wav");
	sounds[SFX_DECLINE] = Mix_LoadWAV("sound/ui/SFX_Decline.wav");
}

//loads the music file referred to with "filename".
//pass in NULL for "filename" to unload the current music file without loading a new one
void loadMusic(char *filename) {
	//unload any music that may be currently loaded
	if (music != NULL) {
		Mix_HaltMusic(music);		//stop playback
		Mix_FreeMusic(music);	//deallocate
		music = NULL;				//eliminate dangling pointer
	}

	//load new music if new music was given
	if(filename != NULL)
		music = Mix_LoadMUS(filename);
}

//wrapper for Mix_PlayMusic
//music can either be looped indefinitely ("loop" = true) or played once ("loop" = false)
void playMusic(bool loop) {
	Mix_PlayMusic(music, loop ? -1 : 0);
	setMusicVolume(app.preferences.musicVolume);
}

//set the volume level of the music track currently playing.
//volume is set between 0 and 10, 0 being muted, 10 being the loudest.
void setMusicVolume(int volume) {
	Mix_VolumeMusic(volume / 10.0 * MIX_MAX_VOLUME * MUSIC_VOL_ADJUST);
}

//wrapper for Mix_PlayMusic
//a sound can either be looped indefinitely ("loop" = true) or played once ("loop" = false)
//if a sound is being looped indefinitely and needs to be stopped, call Mix_HaltChannel().
//"panning" is a value from 0 to 255 controlling the panning of the sound (0 for fully left, 255 for fully right, 127 for center)
//pass in 127 to not apply panning (technically the sound will be very slightly panned though)
//notably, this method of panning means that all sounds will be played at half volume (i think)
//panning values clamped between 0 and 255
void playSound(int id, int channel, bool loop, int panning) {
	Mix_SetPanning(channel, MAX(MIN(255 - panning, 255), 0), MAX(MIN(panning, 255), 0));
	Mix_PlayChannel(channel, sounds[id], loop ? -1 : 0);
}

//halts all sounds playing in the channel before playing the specified sound
void playSoundIsolated(int id, int channel, bool loop, int panning) {
	//Mix_HaltChannel() will halt all sounds if -1 / SC_ANY is passed into it
	//i don't want that behavior; therefore, if SC_ANY is passed in,
	//don't halt anything
	if(channel != SC_ANY)
		Mix_HaltChannel(channel);
	Mix_SetPanning(channel, MAX(MIN(255 - panning, 255), 0), MAX(MIN(panning, 255), 0));
	Mix_PlayChannel(channel, sounds[id], loop ? -1 : 0);
}

//set the volume level of all sounds.
//volume is set between 0 and 10, 0 being muted, 10 being the loudest.
void setSoundVolume(int volume) {
	for (int i = 0; i < SFX_MAX; ++i) {
		Mix_VolumeChunk(sounds[i], volume / 10.0 * MIX_MAX_VOLUME * SOUND_VOL_ADJUST);
	}
}

//cleanup.
void deleteSounds(void) {
	//unload music
	loadMusic(NULL);
	music = NULL;

	//unload sounds
	for (int i = 0; i < MAX_SOUND_CHANNELS; ++i) {
		Mix_FreeChunk(sounds[i]);
		sounds[i] = NULL;
	}
}