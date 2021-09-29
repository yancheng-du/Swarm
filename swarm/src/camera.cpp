#include <cassert>
#include <cstring>
#include <mutex>
#include <thread>
#include <libfreenect.h>
#include <SDL_log.h>

#include <opencv2/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "camera.h"

static void kinect_thread_function();
static void kinect_video_callback(freenect_device *device, void *buffer, uint32_t timestamp);
static void kinect_depth_callback(freenect_device *device, void *buffer, uint32_t timestamp);

static freenect_context *g_kinect_context= NULL;
static freenect_device *g_kinect_device= NULL;
static bool g_kinect_thread_run= false;
static std::thread *g_kinect_thread= NULL;

static std::mutex g_frame_mutex;
static video_frame_t g_video_frame= {0};
static depth_frame_t g_depth_frame= {0};
static int g_frame_count= 0;

bool camera_initialize()
{
	bool success= false;

	if (freenect_init(&g_kinect_context, NULL)==0)
	{
		freenect_device_attributes *device_attributes= NULL;

		#ifdef DEBUG
		freenect_set_log_level(g_kinect_context, FREENECT_LOG_DEBUG);
		#endif

		freenect_select_subdevices(g_kinect_context, (freenect_device_flags)(FREENECT_DEVICE_MOTOR|FREENECT_DEVICE_CAMERA));

		if (freenect_list_device_attributes(g_kinect_context, &device_attributes)>0)
		{
			if (freenect_open_device_by_camera_serial(g_kinect_context, &g_kinect_device, device_attributes->camera_serial)==0)
			{
				if (freenect_set_video_mode(g_kinect_device, freenect_find_video_mode(FREENECT_RESOLUTION_MEDIUM, FREENECT_VIDEO_RGB))==0)
				{
					#ifdef DEBUG
					freenect_frame_mode video_mode= freenect_get_current_video_mode(g_kinect_device);
					assert(video_mode.is_valid);
					assert(video_mode.video_format==FREENECT_VIDEO_RGB);
					assert(video_mode.width==k_camera_width);
					assert(video_mode.height==k_camera_height);
					assert(video_mode.bytes==sizeof(video_frame_t));
					#endif

					if (freenect_set_depth_mode(g_kinect_device, freenect_find_depth_mode(FREENECT_RESOLUTION_MEDIUM, FREENECT_DEPTH_REGISTERED))==0)
					{
						#ifdef DEBUG
						freenect_frame_mode depth_mode= freenect_get_current_depth_mode(g_kinect_device);
						assert(depth_mode.is_valid);
						assert(depth_mode.depth_format==FREENECT_DEPTH_REGISTERED);
						assert(depth_mode.width==k_camera_width);
						assert(depth_mode.height==k_camera_height);
						assert(depth_mode.bytes==sizeof(depth_frame_t));
						#endif

						freenect_set_video_callback(g_kinect_device, kinect_video_callback);
						freenect_set_depth_callback(g_kinect_device, kinect_depth_callback);

						if (freenect_start_video(g_kinect_device)==0 &&
							freenect_set_flag(g_kinect_device, FREENECT_MIRROR_VIDEO, FREENECT_ON)==0)
						{
							if (freenect_start_depth(g_kinect_device)==0 &&
								freenect_set_flag(g_kinect_device, FREENECT_MIRROR_DEPTH, FREENECT_ON)==0)
							{
								g_kinect_thread_run= true;
								g_kinect_thread= new std::thread(kinect_thread_function);

								success= true;
							}
							else
							{
								SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't start depth streaming");
							}
						}
						else
						{
							SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't start video streaming");
						}
					}
					else
					{
						SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't set depth mode");
					}
				}
				else
				{
					SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't set video mode");
				}
			}
			else
			{
				SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't open kinect device");
			}

			freenect_free_device_attributes(device_attributes);
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
	if (g_kinect_thread)
	{
		g_kinect_thread_run= false;
		g_kinect_thread->join();
		delete g_kinect_thread;
		g_kinect_thread= NULL;
	}

	if (g_kinect_device)
	{
		freenect_stop_depth(g_kinect_device);
		freenect_stop_video(g_kinect_device);

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
	video_frame_t *video_frame,
	depth_frame_t *depth_frame,
	edge_frame_t *edge_frame)
{
	int frame_count;

	g_frame_mutex.lock();
	std::memcpy(video_frame, g_video_frame, sizeof(*video_frame));
	std::memcpy(depth_frame, g_depth_frame, sizeof(*depth_frame));
	frame_count= g_frame_count;
	g_frame_mutex.unlock();

	cv::Mat original, grey, blurred, edges;
	original = cv::Mat(k_camera_height, k_camera_width, CV_8UC3, (uint8_t*)video_frame);
	cv::cvtColor(original, grey, cv::COLOR_BGR2GRAY);
	cv::GaussianBlur(grey,            // input image
		blurred,                      // output image
		cv::Size(3, 3),               // smoothing window width and height in pixels
		2);							  //sigma
	cv::Canny(blurred,				  // input image
		edges,						  // output image
		100,                          // low threshold
		250);
	std::memcpy(edge_frame, edges.data, sizeof(*edge_frame));

	return frame_count;
}



static void kinect_thread_function()
{
	if (freenect_set_led(g_kinect_device, LED_GREEN)!=0)
	{
		SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Couldn't set kinect LED");
	}

	while (g_kinect_thread_run)
	{
		if (freenect_process_events(g_kinect_context)!=0)
		{
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't process USB events");
		}
	}

	if (freenect_set_led(g_kinect_device, LED_RED)!=0)
	{
		SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Couldn't set kinect LED");
	}
}

static void kinect_video_callback(freenect_device *device, void *buffer, uint32_t timestamp)
{
	g_frame_mutex.lock();
	memcpy(&g_video_frame, buffer, sizeof(g_video_frame));
	g_frame_count++;
	g_frame_mutex.unlock();
}

static void kinect_depth_callback(freenect_device *device, void *buffer, uint32_t timestamp)
{
	g_frame_mutex.lock();
	memcpy(&g_depth_frame, buffer, sizeof(g_depth_frame));
	g_frame_mutex.unlock();
}
