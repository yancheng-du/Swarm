#include <cstdlib>
#include <SDL.h>
#include <thread>

#include "camera.h"
#include "constants.h"
#include "graphics.h"
#include "swarm.h"

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
		counter=SDL_GetPerformanceCounter();
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
		bool idle = false;
		bool fps= false;
		bool debug= false;

		timer_t timer;

		swarm_t swarm;
		cv::Mat3b video_frame;
		cv::Mat3b last_video_frame;
		cv::Mat1w depth_frame;
		cv::Mat1b edge_frame;
		

		#ifdef DEBUG
		SDL_LogSetAllPriority(SDL_LOG_PRIORITY_WARN);
		#endif

		camera_initialize();
		graphics_initialize();


		while (running)
		{
			timer.reset();
			// process frame
			camera_read_frame(&video_frame, &depth_frame, &edge_frame,idle);
			swarm.update(&edge_frame);
			graphics_render(&swarm, fps, debug, &video_frame, &depth_frame, &edge_frame);
			idle_check(&video_frame, &last_video_frame,&idle);

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
			while (!timer.passed(1.0/k_fps));
		}

		graphics_dispose();
		camera_dispose();
		SDL_Quit();
	}
	else
	{
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize SDL: %s", SDL_GetError());
		result= EXIT_FAILURE;
	}

	return result;
}
