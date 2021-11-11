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

static cv::Mat g_idle_image= cv::imread("res/ece.bmp", cv::IMREAD_GRAYSCALE); // Load idle image

static int image_dist_counter= k_fps/idle_checks_per_sec - 1;
static int idle_check_counter= (int)(seconds_before_idle*k_fps/(image_dist_counter)-1);
static float running_avg= 0.0f;
static float last_running_avg= 0.0f;

bool director_initialize()
{
	graphics_initialize();
	audio_initialize();
	camera_initialize();
	gesture_initialize();

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
		g_idle_image.copyTo(g_edge_frame);
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
	if (image_dist_counter==0)
	{
		image_dist_counter= k_fps/idle_checks_per_sec - 1;
		int dist= image_dist(video_frame, last_video_frame);
		running_avg= running_avg_alpha*dist + (1-running_avg_alpha)*running_avg;
		if (dist>running_avg*1.5)
		{
			idle= false;
			idle_check_counter= (int)(seconds_before_idle*k_fps/(image_dist_counter)-1);
		}
		if (idle_check_counter<=0)
		{
			idle_check_counter= (int)(seconds_before_idle*k_fps/(image_dist_counter)-1);
			if (running_avg<last_running_avg*1.1 && dist<last_running_avg*1.1)
			{
				idle= true;
			}
			last_running_avg= running_avg;
		}
		idle_check_counter-= 1;
	}
	else
	{
		image_dist_counter-= 1;
	}
}

static int image_dist(const cv::Mat3b &video_frame, cv::Mat3b &last_video_frame)
{
	last_video_frame.create(k_camera_height, k_camera_width);
	int diff= (int)cv::norm(video_frame-last_video_frame);
	last_video_frame= video_frame.clone();
	return diff;
}
