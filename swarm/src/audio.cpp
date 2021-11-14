#include <SDL_log.h>
#include <SDL_mixer.h>

#include "audio.hpp"
#include "swarm.hpp"

Mix_Chunk *buzz_wav[bee_t::k_state_count];
const char* wavfile_names[] =
{
	"res/wavfiles/swarm_base.wav",
	"res/wavfiles/swarm_low.wav",
	"res/wavfiles/swarm_high.wav"
};
const float channel_control[] =
{
	0.6,
	0.3,
	0.4
};

const int b_max_change = 32; // [0~128]

bool audio_initialize()
{
	int success= Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
	if(success < 0)
	{
		SDL_LogError(SDL_LOG_CATEGORY_AUDIO, "Couldn't initialize audio mixer: %s", Mix_GetError());
	}

	success = Mix_AllocateChannels(bee_t::k_state_count);
	if(success < 0)
	{
		SDL_LogError(SDL_LOG_CATEGORY_AUDIO, "Couldn't allocate channels: %s", Mix_GetError());
	}

	for(int i=0; i<bee_t::k_state_count; i++)
	{
		buzz_wav[i]= Mix_LoadWAV(wavfile_names[i]);
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
	for(int i=0; i<bee_t::k_state_count; i++)
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
	int mix_volume= 64;
	static float prev_volume[bee_t::k_state_count];
	static float new_volume[bee_t::k_state_count];

	for (int i=0; i<bee_t::k_state_count; i++)
	{
		if(i==0) // _idle
		{
			mix_volume= 128*(.9 - swarm.state_fractions[i])*(.9 - swarm.state_fractions[i]);
		}
		new_volume[i]= channel_control[i]*mix_volume*(1 - swarm.state_fractions[i]);
		float change= new_volume[i] - prev_volume[i];
		if (std::abs(change)>b_max_change)
		{
			if (change<0)
			{
				new_volume[i]= prev_volume[i]-b_max_change;
			}
			else
			{
				new_volume[i]= prev_volume[i]+b_max_change;
			}
		}
		Mix_Volume(i, new_volume[i]);
		// play audio
		if (Mix_Playing(i)==0)
		{
			Mix_PlayChannel(i, buzz_wav[i], -1);
		}
		for (int i=0; i<bee_t::k_state_count; i++)
		{
			prev_volume[i]= new_volume[i];
		}
	}
}
