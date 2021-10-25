#include <SDL_mixer.h>

#include "audio.hpp"
#include "camera.hpp"
#include "swarm.hpp"

Mix_Chunk *buzz_wav[3];
const char* wavfile_names[] =
{
    "res/wavfiles/swarm_low.wav",
    "res/wavfiles/swarm_base.wav",
    "res/wavfiles/swarm_high.wav"
};

bool audio_initialize()
{
    int success= Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
    if(success < 0)
    {
        SDL_LogError(SDL_LOG_CATEGORY_AUDIO, "Couldn't initialize audio mixer: %s", Mix_GetError());
    }
    
    success = Mix_AllocateChannels(3);
    if(success < 0)
    {
        SDL_LogError(SDL_LOG_CATEGORY_AUDIO, "Couldn't allocate channels: %s", Mix_GetError());
    }
    /*
    for(int i=0; i<sizeof(buzz_wav); i++)
    {
        buzz_wav[i]= Mix_LoadWAV(wavfile_names[i]);
    }
     */
    buzz_wav[0]= Mix_LoadWAV(wavfile_names[0]);
    buzz_wav[1]= Mix_LoadWAV(wavfile_names[1]);
    buzz_wav[2]= Mix_LoadWAV(wavfile_names[2]);
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
    for(int i=0; i<sizeof(buzz_wav); i++)
    {
        if (buzz_wav[i])
        {
            Mix_FreeChunk(buzz_wav[i]);
            buzz_wav[i]=NULL;
        }
    }
	

	while (Mix_Init(0))
	{
		Mix_Quit();
	}
}

void audio_render(const swarm_t &swarm)
{
    float idle_vol= swarm.state_fractions[0];
    float crawl_vol= swarm.state_fractions[1];
    float flying_vol= swarm.state_fractions[2];
    int mix_volume= (get_distance()/get_avg_distance())*64;
    
    // low buzz (idle)
    Mix_Volume(0, mix_volume*idle_vol);
    if (Mix_Playing(0)==0)
    {
        Mix_PlayChannel(0, buzz_wav[0], -1);
    }
    // base buzz (crawl)
    Mix_Volume(1, mix_volume);
    if (Mix_Playing(1)==0)
    {
        Mix_PlayChannel(1, buzz_wav[1], -1);
    }
    
    // high buzz (flying)
    Mix_Volume(2, mix_volume*flying_vol);
    if (Mix_Playing(2)==0)
    {
        Mix_PlayChannel(2, buzz_wav[2], -1);
    }
}
