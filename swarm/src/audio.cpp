#include <SDL_mixer.h>

#include "audio.hpp"
#include "camera.hpp"

// Continuous sounds
Mix_Music *buzz= NULL;
// Sound effects

bool load_media()
{
    buzz= Mix_LoadMUS("res/wavfiles/beeswarm_mid.mp3");
    if(buzz == NULL)
    {
        printf("%s\n", Mix_GetError());
        return false;
    }
    return true;
}

bool audio_initialize()
{
	bool success= false;
	if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048)  >= 0)
	{
        success= true;
	}
    else
    {
        SDL_LogError(SDL_LOG_CATEGORY_AUDIO, "Couldn't initialize audio mixer: %s", Mix_GetError());
        success= false;
    }
    if(load_media())
    {
        success= true;
    }
    else
    {
        success= false;
    }

	return success;
}

void audio_dispose()
{
	// $TODO reset globals and free sounds
    Mix_FreeMusic(buzz);
    buzz=NULL;
	while (Mix_Init(0))
	{
		Mix_Quit();
	}
}

void audio_render(const swarm_t *swarm)
{
	// $TODO analyze bees, start, update, and stop sounds
    // volume control
    int volume= (get_distance()/get_avg_distance())*64;
    Mix_VolumeMusic(volume);
    if(Mix_PlayingMusic() == 0)
    {
        Mix_PlayMusic(buzz, -1);
    }
}
