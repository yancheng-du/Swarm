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

static const char *k_idle_image_filepaths[]=
{
	"res/bevo.bmp",
	"res/ece.bmp",
	"res/Texas.bmp",
	"res/title.bmp",
	"res/tower.bmp"
};
static const int k_idle_image_count= sizeof(k_idle_image_filepaths)/sizeof(k_idle_image_filepaths[0]);

static const int k_idle_checks_per_sec= 5;
static const float k_idle_running_avg_alpha= 2.0f/(k_seconds_before_idle*k_idle_checks_per_sec+1.0f);

static void director_idle_check(const cv::Mat3b &video_frame, cv::Mat3b &last_video_frame, bool &idle);
static int image_dist(const cv::Mat3b &video_frame, cv::Mat3b &last_video_frame);

static bool g_running= true;
static bool g_fullscreen= false;
static bool g_idle= false;
static bool g_debug= false;
static bool g_fps= false;

static cv::Mat3b g_video_frame;
static cv::Mat3b g_last_video_frame;
static cv::Mat1w g_depth_frame;
static cv::Mat1b g_edge_frame;
static commands_t g_commands;

static swarm_t g_swarm;

static int g_idle_image_index= -1;
static cv::Mat g_idle_images[k_idle_image_count];
static int g_image_dist_counter= k_fps/k_idle_checks_per_sec - 1;
static int g_idle_check_counter= k_seconds_before_idle*k_idle_checks_per_sec - 1;
static float g_idle_running_avg= 0.0f;
static float g_last_running_avg= 0.0f;

bool director_initialize()
{
	graphics_initialize();
	audio_initialize();
	camera_initialize();
	gesture_initialize();

	for (int idle_image_index= 0; idle_image_index<k_idle_image_count; idle_image_index++)
	{
		g_idle_images[idle_image_index]= cv::imread(k_idle_image_filepaths[idle_image_index], cv::IMREAD_GRAYSCALE);
	}

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
	director_idle_check(g_video_frame, g_last_video_frame, g_idle);
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
						g_image_dist_counter= k_fps/k_idle_checks_per_sec - 1;
						g_idle_check_counter= k_seconds_before_idle*k_idle_checks_per_sec - 1;
						if (g_idle)
						{
							g_idle_image_index= (g_idle_image_index+1)%k_idle_image_count;
						}
						break;
					}

					// $TODO add other key handling events here
				}
				break;
			}
		}
	}
}

static void director_idle_check(const cv::Mat3b &video_frame, cv::Mat3b &last_video_frame, bool &idle)
{
	if (g_image_dist_counter<=0)
	{
		g_image_dist_counter= k_fps/k_idle_checks_per_sec - 1;
		int dist= image_dist(video_frame, last_video_frame);
		g_idle_running_avg= k_idle_running_avg_alpha*dist + (1-k_idle_running_avg_alpha)*g_idle_running_avg;
		if (dist>g_idle_running_avg*1.5)
		{
			idle= false;
			g_idle_check_counter= k_seconds_before_idle*k_idle_checks_per_sec - 1;
		}
		if (g_idle_check_counter<=0)
		{
			g_idle_check_counter= k_seconds_before_idle*k_idle_checks_per_sec - 1;
			if (g_idle_running_avg<g_last_running_avg*1.1 && dist<g_last_running_avg*1.1)
			{
				int last_idle_image_index= g_idle_image_index;
				while (g_idle_image_index==last_idle_image_index)
				{
					g_idle_image_index= rand()%k_idle_image_count;
				}
				idle= true;
			}
			g_last_running_avg= g_idle_running_avg;
		}
		g_idle_check_counter-= 1;
	}
	else
	{
		g_image_dist_counter-= 1;
	}
}

static int image_dist(const cv::Mat3b &video_frame, cv::Mat3b &last_video_frame)
{
	last_video_frame.create(k_camera_height, k_camera_width);
	int diff= (int)cv::norm(video_frame-last_video_frame);
	last_video_frame= video_frame.clone();
	return diff;
}
