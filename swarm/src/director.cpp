
#include "audio.hpp"
#include "camera.hpp"
#include "constants.hpp"
#include "director.hpp"
#include "gesture.hpp"
#include "graphics.hpp"

director_t::director_t()
{
	graphics_initialize();
	audio_initialize();
	camera_initialize();
	gesture_initialize();
}

director_t::~director_t()
{
	gesture_dispose();
	camera_dispose();
	audio_dispose();
	graphics_dispose();
}

bool director_t::is_running()
{
	return running;
}

void director_t::do_frame()
{
	camera_consume_full_frame(video_frame, depth_frame, edge_frame, idle);
	idle_check(video_frame, last_video_frame, idle);
	gesture_consume_commands(commands);
	swarm.update(edge_frame,commands);
	graphics_render(swarm, debug, video_frame, depth_frame, edge_frame, fps);
	audio_render(swarm);
}

void director_t::process_events()
{
	SDL_Event event;

	while (SDL_PollEvent(&event))
	{
		SDL_LogInfo(SDL_LOG_CATEGORY_INPUT, "Received event type 0x%x", event.type);

		switch (event.type)
		{
			case SDL_QUIT:
			{
				running= false;
				break;
			}

			case SDL_KEYDOWN:
			{
				SDL_Keycode code= event.key.keysym.sym;

				switch (code)
				{
					case SDLK_ESCAPE:
					{
						running= false;
						break;
					}

					case SDLK_LCTRL:
					{
						fps= !fps;
						break;
					}

					case SDLK_LALT:
					{
						debug= !debug;
						break;
					}

					case SDLK_i:
					{
						idle= !idle;
						break;
					}

					// $TODO add other key handling events here
				}
				break;
			}
		}
	}
}
