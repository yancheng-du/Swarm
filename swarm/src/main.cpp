#include <SDL.h>

#include "camera.h"
#include "graphics.h"

int main(int argc, char *argv[])
{
	if (SDL_Init(SDL_INIT_VIDEO)==0)
	{
		camera_initialize();
		graphics_initialize();
		
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

				camera_read_frame(&video_frame, &depth_frame);
				graphics_render(&video_frame, &depth_frame);
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
