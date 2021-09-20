#include <SDL.h>

#include "camera.h"
#include "graphics.h"
#include "Swarm.h"

int main(int argc, char *argv[])
{	

	if (SDL_Init(SDL_INIT_VIDEO)==0)
	{
		camera_initialize();
		graphics_initialize();

		Swarm bee_swarm = Swarm();
		bee_swarm.swarm_init();
		
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
				edge_frame_t edge_frame;
				video_frame_t video_frame;
				depth_frame_t depth_frame;

				camera_read_frame(&video_frame, &depth_frame, &edge_frame);
				bee_swarm.swarm_update(edge_frame);
				draw_bees(&bee_swarm);
				//graphics_render(&video_frame, &depth_frame);
				
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
