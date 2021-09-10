#include <cstring>
#include <libfreenect.h>
#include <SDL_log.h>

#include "camera.h"

static freenect_context *g_kinect_context= NULL;
static freenect_device *g_kinect_device= NULL;

static int g_frame_count= 0;

static RGB_frame_t g_RGB_frames[2]= {{0}, {0}};
static int g_last_RGB_frame_index= 0;

static depth_frame_t g_depth_frames[2]= {{0}, {0}};
static int g_last_depth_frame_index= 0;

bool camera_initialize()
{
	bool success= false;

	if (freenect_init(&g_kinect_context, NULL)==0)
	{
		if (freenect_num_devices(g_kinect_context)>0)
		{
			if (freenect_open_device(g_kinect_context, &g_kinect_device, 0)==0)
			{
				if (freenect_set_led(g_kinect_device, LED_GREEN)!=0)
				{
					SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Couldn't set kinect LED");
				}

				success= true;
			}
			else
			{
				SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't open kinect device");
			}
		}
		else
		{
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't find any kinect devices");
		}
	}
	else
	{
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create kinect context");
	}


	return success;
}

void camera_dispose()
{
	if (g_kinect_device)
	{
		if (freenect_set_led(g_kinect_device, LED_RED)!=0)
		{
			SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Couldn't set kinect LED");
		}

		freenect_close_device(g_kinect_device);
		g_kinect_device= NULL;

	}

	if (g_kinect_context)
	{
		freenect_shutdown(g_kinect_context);
		g_kinect_context= NULL;
	}
}

int camera_read_frame(
	RGB_frame_t *RGB_frame,
	depth_frame_t *depth_frame)
{
	std::memcpy(RGB_frame, &g_RGB_frames[g_last_RGB_frame_index], sizeof(*RGB_frame));
	std::memcpy(depth_frame, &g_depth_frames[g_last_depth_frame_index], sizeof(*depth_frame));

	return g_frame_count;
}
