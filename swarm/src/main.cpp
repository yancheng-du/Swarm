#include <cstdlib>

#include <SDL.h>

#include "audio.hpp"
#include "camera.hpp"
#include "constants.hpp"
#include "gesture.hpp"
#include "graphics.hpp"
#include "swarm.hpp"

class timer_t
{
public:
	timer_t()
	{
		frequency= static_cast<double>(SDL_GetPerformanceFrequency());
		reset();
	}

	void reset()
	{
		counter= SDL_GetPerformanceCounter();
	}

	bool passed(double time)
	{
		return (SDL_GetPerformanceCounter()-counter)/frequency>time;
	}

private:
	double frequency;
	uint64_t counter;
};

int main(int argc, char *argv[])
{
	int result= EXIT_SUCCESS;

	if (SDL_Init(SDL_INIT_AUDIO|SDL_INIT_VIDEO|SDL_INIT_EVENTS)==0)
	{
		bool running= true;
		bool idle= false;
		bool fps= false;
		bool debug= false;

		timer_t timer;

		cv::Mat3b video_frame;
		cv::Mat3b last_video_frame;
		cv::Mat1w depth_frame;
		cv::Mat1b edge_frame;
		commands_t commands;

		swarm_t swarm;

		#ifdef DEBUG
		SDL_LogSetAllPriority(SDL_LOG_PRIORITY_WARN);
		#endif

		graphics_initialize();
		audio_initialize();
		camera_initialize();
		gesture_initialize();

		while (running)
		{
			timer.reset();

			// process frame
			camera_consume_full_frame(&video_frame, &depth_frame, &edge_frame, idle);
			idle_check(&video_frame, &last_video_frame,&idle);
			gesture_consume_commands(commands);
			swarm.update(&edge_frame);
			graphics_render(&swarm, fps, debug, &video_frame, &depth_frame, &edge_frame);
			audio_render(&swarm);

			// process events while waiting to start next frame
			do
			{
				SDL_Event event;

				while (SDL_PollEvent(&event))
				{
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

								case SDLK_i: // toggle idle image
								{
									idle = !idle;
									break;
								}

								// $TODO add other key handling events here
							}
							break;
						}

						default:
						{
							SDL_LogInfo(SDL_LOG_CATEGORY_INPUT, "Received event type 0x%x", event.type);
						}
					}
				}
			}
			while (!timer.passed(k_dt));
		}

		gesture_dispose();
		camera_dispose();
		audio_dispose();
		graphics_dispose();

		SDL_Quit();
	}
	else
	{
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize SDL: %s", SDL_GetError());
		result= EXIT_FAILURE;
	}

	return result;
}
