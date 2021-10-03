#include <SDL.h>

#include "camera.h"
#include "constants.h"
#include "graphics.h"
#include "swarm.h"
#include <tensorflow/c/c_api.h>

int main(int argc, char *argv[])
{
	if (SDL_Init(SDL_INIT_VIDEO)==0)
	{
		camera_initialize();
		graphics_initialize();

		swarm_t swarm;

		while (true)
		{
			SDL_Event event;

			SDL_PollEvent(&event);

			if (event.type==SDL_QUIT)
			{
				break;
			}
			else
			{
				video_frame_t video_frame;
				depth_frame_t depth_frame;
				edge_frame_t edge_frame;

				camera_read_frame(&video_frame, &depth_frame, &edge_frame);
				swarm.update(edge_frame, 1.0f/k_fps);
				graphics_render(&video_frame, &depth_frame, &edge_frame, &swarm);
			}
		}

		graphics_dispose();
		camera_dispose();
	}
	else
	{
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize SDL: %s", SDL_GetError());
	}

	return 0;
}
