#include <SDL_mixer.h>

#include "audio.hpp"

bool audio_initialize()
{
	bool success= false;

	if (Mix_Init(MIX_INIT_MP3)==MIX_INIT_MP3)
	{
		// $TODO load sounds and setup globals

		success= true;
	}
	else
	{
		SDL_LogError(SDL_LOG_CATEGORY_AUDIO, "Couldn't initialize audio mixer: %s", Mix_GetError());
	}

	return success;
}

void audio_dispose()
{
	// $TODO reset globals and free sounds

	while (Mix_Init(0))
	{
		Mix_Quit();
	}
}

void audio_render(const swarm_t *swarm)
{
	// $TODO analyze bees, start, update, and stop sounds
}
