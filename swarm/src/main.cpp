#include <cstdlib>

#include <SDL.h>

#include "constants.hpp"
#include "director.hpp"
#include "timer.hpp"

int main(int argc, char *argv[])
{
	int result= EXIT_SUCCESS;

	#ifdef DEBUG
	SDL_LogSetAllPriority(SDL_LOG_PRIORITY_WARN);
	#endif

	if (SDL_Init(SDL_INIT_AUDIO|SDL_INIT_VIDEO|SDL_INIT_EVENTS)==0)
	{
		if (director_initialize())
		{
			timer_t timer;

			while (director_is_running())
			{
				timer.reset();
				director_do_frame();
				do director_process_events(); while (!timer.passed(k_dt));
			}

			director_dispose();
		}
		else
		{
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize director");
		}

		SDL_Quit();
	}
	else
	{
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize SDL: %s", SDL_GetError());
		result= EXIT_FAILURE;
	}

	return result;
}
