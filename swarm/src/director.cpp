#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <SDL_events.h>
#include <SDL_log.h>

#include "audio.hpp"
#include "camera.hpp"
#include "constants.hpp"
#include "director.hpp"
#include "gesture.hpp"
#include "graphics.hpp"
#include "swarm.hpp"
#include "timer.hpp"

static const double k_idle_maximum_image_distance= 0.01*k_edge_width*k_edge_height*UINT8_MAX;

static const char *k_idle_image_filepaths[]=
{
	"res/bevo.bmp",
	"res/ece.bmp",
	"res/Texas.bmp",
	"res/title.bmp",
	"res/tower.bmp"
};
static const int k_idle_image_count= sizeof(k_idle_image_filepaths)/sizeof(k_idle_image_filepaths[0]);

static const double k_title_time= 5.0;
static const int k_title_image_index= 3;

static void director_idle_update(int num_gesture);

static bool g_running;
static bool g_fullscreen;
static bool g_idle;
static bool g_debug;
static bool g_fps;

static cv::Mat3b g_video_frame;
static cv::Mat1w g_depth_frame;
static cv::Mat1b g_edge_frame;
static cv::Mat1b g_last_edge_frame;

static commands_t g_commands;

static swarm_t g_swarm;

static int g_idle_image_index;
static cv::Mat g_idle_images[k_idle_image_count];
static timer_t g_idle_timer;

bool director_initialize()
{
	graphics_initialize();
	audio_initialize();
	camera_initialize();
	gesture_initialize();

	g_running= true;
	g_fullscreen= false;
	g_idle= true;
	g_debug= false;
	g_fps= false;
	
	g_last_edge_frame= cv::Mat::zeros(k_edge_height, k_edge_width, CV_8U);

	g_idle_image_index= k_title_image_index;
	for (int idle_image_index= 0; idle_image_index<k_idle_image_count; idle_image_index++)
	{
		g_idle_images[idle_image_index]= cv::imread(k_idle_image_filepaths[idle_image_index], cv::IMREAD_GRAYSCALE);
	}
	g_idle_timer.start(k_title_time);

	return true;
}

void director_dispose()
{
	gesture_dispose();
	camera_dispose();
	audio_dispose();
	graphics_dispose();
}

bool director_is_running()
{
	return g_running;
}

void director_do_frame()
{
	camera_consume_full_frame(g_video_frame, g_depth_frame, g_edge_frame);
	director_idle_update(g_commands.size());
	if (g_idle)
	{
		g_idle_images[g_idle_image_index].copyTo(g_edge_frame);

	}
	gesture_consume_commands(g_commands);
	g_swarm.update(g_edge_frame, g_commands);
	graphics_render(g_swarm, g_debug, g_video_frame, g_depth_frame, g_edge_frame, g_commands, g_fps);
	audio_render(g_swarm);
}

void director_process_events()
{
	SDL_Event event;

	while (SDL_PollEvent(&event))
	{
		SDL_LogInfo(SDL_LOG_CATEGORY_INPUT, "Received event type 0x%x", event.type);

		switch (event.type)
		{
			case SDL_QUIT:
			{
				g_running= false;
				break;
			}

			case SDL_KEYDOWN:
			{
				SDL_Keycode code= event.key.keysym.sym;

				switch (code)
				{
					case SDLK_ESCAPE:
					{
						g_running= false;
						break;
					}

					case SDLK_LCTRL:
					{
						g_fps= !g_fps;
						break;
					}

					case SDLK_LALT:
					{
						g_debug= !g_debug;
						break;
					}

					case SDLK_a:
					{
						g_swarm.flow_active= !g_swarm.flow_active;
						break;
					}
					case SDLK_f:
					{
						if (graphics_change_mode(!g_fullscreen))
						{
							g_fullscreen= !g_fullscreen;
						}
						break;
					}

					case SDLK_i:
					{
						g_idle= !g_idle;
						if (g_idle)
						{
							g_idle_image_index= (g_idle_image_index+1)%k_idle_image_count;
							g_idle_timer.start(1);
						}
						else
						{
							g_idle_timer.reset();
						}
						break;
					}

					case SDLK_t:
					{
						g_idle= true;
						g_idle_image_index= k_title_image_index;
						g_idle_timer.start(k_title_time);
						g_swarm.reset();
						break;
					}

					// $TODO add other key handling events here
				}
				break;
			}
		}
	}
}

static void director_idle_update(int num_gestures)
{
	if (num_gestures>0) {
		g_idle= false;
		g_idle_timer.reset();
	}
	else
	{
		double distance= cv::norm(g_last_edge_frame, g_edge_frame, cv::NORM_L1);
		if (!g_idle_timer.running())
		{
			if (distance>k_idle_maximum_image_distance)
			{
				g_idle= false;
				g_idle_timer.reset();
			}
			else if (g_idle_timer.passed(k_seconds_before_idle))
			{
				g_idle= true;

				int last_idle_image_index= g_idle_image_index;
				while (g_idle_image_index==last_idle_image_index)
				{
					g_idle_image_index= rand()%k_idle_image_count;
				}
				g_idle_timer.reset();
			}
		}
		g_edge_frame.copyTo(g_last_edge_frame);
	}
}
