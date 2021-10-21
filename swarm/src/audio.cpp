#include <SDL_mixer.h>

#include "audio.hpp"
#include "camera.hpp"

Mix_Music *g_buzz_sound= NULL;

bool audio_initialize()
{
	bool success= false;

	if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048)  >= 0)
	{
		g_buzz_sound= Mix_LoadMUS("res/wavfiles/beeswarm_mid.mp3");

		if (g_buzz_sound)
		{
			success= true;
		}
		else
		{
			SDL_LogError(SDL_LOG_CATEGORY_AUDIO, "Couldn't load audio: %s", Mix_GetError());
		}
	}
	else
	{
		SDL_LogError(SDL_LOG_CATEGORY_AUDIO, "Couldn't initialize audio mixer: %s", Mix_GetError());
	}

	return success;
}

void audio_dispose()
{
	if (g_buzz_sound)
	{
		Mix_FreeMusic(g_buzz_sound);
		g_buzz_sound=NULL;
	}

	while (Mix_Init(0))
	{
		Mix_Quit();
	}
}

void audio_render(const swarm_t &swarm)
{
	if (g_buzz_sound)
	{
		int volume= (get_distance()/get_avg_distance())*64;

		Mix_VolumeMusic(volume);

		if (Mix_PlayingMusic()==0)
		{
			Mix_PlayMusic(g_buzz_sound, -1);
		}
	}
}
