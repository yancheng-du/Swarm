#include <SDL_mixer.h>

#include "audio.hpp"
#include "camera.hpp"

Mix_Chunk *g_buzz_sound= NULL;

bool audio_initialize()
{
    int success= Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
    if(success < 0)
    {
        SDL_LogError(SDL_LOG_CATEGORY_AUDIO, "Couldn't initialize audio mixer: %s", Mix_GetError());
        exit(-1);
    }
    
    success = Mix_AllocateChannels(3);
    if(success < 0)
    {
        SDL_LogError(SDL_LOG_CATEGORY_AUDIO, "Couldn't allocate channels: %s", Mix_GetError());
        exit(-1);
    }
    
    g_buzz_sound= Mix_LoadWAV("res/wavfiles/swarm_base.wav");
    if(success < 0)
    {
        SDL_LogError(SDL_LOG_CATEGORY_AUDIO, "Couldn't load WAV file: %s", Mix_GetError());
        exit(-1);
    }
    
    if(success < 0)
    {
        return false;
    }
    else
    {
        return true;
    }
}

void audio_dispose()
{
	if (g_buzz_sound)
	{
		Mix_FreeChunk(g_buzz_sound);
		g_buzz_sound=NULL;
	}

	while (Mix_Init(0))
	{
		Mix_Quit();
	}
}

void audio_render(const swarm_t &swarm)
{
	//if (g_buzz_sound)
	//{
		int volume= (get_distance()/get_avg_distance())*64;

        // basic buzz
        Mix_Volume(1, volume);
		if (Mix_Playing(1)==0)
		{
			Mix_PlayChannel(1, g_buzz_sound, -1);
		}
	//}
}
