#include <cassert>
#include <cstring>
#include <mutex>
#include <thread>

#include <libfreenect.h>
#include <opencv2/imgproc.hpp>
#include <SDL_log.h>

#include "camera.hpp"
#include "constants.hpp"

static void kinect_thread_function();
static void kinect_video_callback(freenect_device *device, void *buffer, uint32_t timestamp);
static void kinect_depth_callback(freenect_device *device, void *buffer, uint32_t timestamp);

static void camera_process_frame(cv::Mat3b &video_frame, cv::Mat1w &depth_frame, cv::Mat1b &edge_frame);

static freenect_context *g_kinect_context= NULL;
static freenect_device *g_kinect_device= NULL;

static bool g_kinect_thread_run= false;
static std::thread *g_kinect_thread= NULL;

static std::mutex g_frame_mutex;
static uint8_t g_video_frame[k_camera_width*k_camera_height*3]= {0};
static uint16_t g_depth_frame[k_camera_width*k_camera_height]= {0};
static int g_frame_count= 0;

// for audio
int distance;
float avg_distance;

int get_distance()
{
	return distance;
}
float get_avg_distance()
{
	return avg_distance;
}

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
					assert(video_mode.bytes==sizeof(g_video_frame));
					#endif

					freenect_set_video_callback(g_kinect_device, kinect_video_callback);

					if (freenect_set_depth_mode(g_kinect_device, freenect_find_depth_mode(FREENECT_RESOLUTION_MEDIUM, FREENECT_DEPTH_REGISTERED))==0)
					{
						#ifdef DEBUG
						freenect_frame_mode depth_mode= freenect_get_current_depth_mode(g_kinect_device);
						assert(depth_mode.is_valid);
						assert(depth_mode.depth_format==FREENECT_DEPTH_REGISTERED);
						assert(depth_mode.width==k_camera_width);
						assert(depth_mode.height==k_camera_height);
						assert(depth_mode.bytes==sizeof(g_depth_frame));
						#endif

						freenect_set_depth_callback(g_kinect_device, kinect_depth_callback);
						//cv::setNumThreads(0);
						g_kinect_thread_run= true;
						g_kinect_thread= new std::thread(kinect_thread_function);

						success= true;
					}
					else
					{
						SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "Couldn't set depth mode");
					}
				}
				else
				{
					SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "Couldn't set video mode");
				}
			}
			else
			{
				SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "Couldn't open kinect device");
			}

			freenect_free_device_attributes(device_attributes);
		}
		else
		{
			SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "Couldn't find any kinect devices");
		}
	}
	else
	{
		SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "Couldn't create kinect context");
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
		freenect_close_device(g_kinect_device);
		g_kinect_device= NULL;
	}

	if (g_kinect_context)
	{
		freenect_shutdown(g_kinect_context);
		g_kinect_context= NULL;
	}
}

void camera_peek_video_frame(
	cv::Mat3b &video_frame)
{
	video_frame.create(k_camera_height, k_camera_width);

	assert(video_frame.isContinuous());
	assert(video_frame.dataend-video_frame.datastart==sizeof(g_video_frame));

	g_frame_mutex.lock();
	std::memcpy(video_frame.data, g_video_frame, sizeof(g_video_frame));
	g_frame_mutex.unlock();
}

int camera_consume_full_frame(
	cv::Mat3b &video_frame,
	cv::Mat1w &depth_frame,
	cv::Mat1b &edge_frame)
{
	int frame_count;

	video_frame.create(k_camera_height, k_camera_width);
	assert(video_frame.isContinuous());
	assert(video_frame.dataend-video_frame.datastart==sizeof(g_video_frame));

	depth_frame.create(k_camera_height, k_camera_width);
	assert(depth_frame.isContinuous());
	assert(depth_frame.dataend-depth_frame.datastart==sizeof(g_depth_frame));

	g_frame_mutex.lock();
	std::memcpy(video_frame.data, g_video_frame, sizeof(g_video_frame));
	std::memcpy(depth_frame.data, g_depth_frame, sizeof(g_depth_frame));
	frame_count= g_frame_count;
	g_frame_mutex.unlock();

	camera_process_frame(video_frame, depth_frame, edge_frame);

	return frame_count;
}

static void kinect_thread_function()
{
	while (freenect_set_led(g_kinect_device, LED_GREEN)!=0)
	{
		SDL_LogWarn(SDL_LOG_CATEGORY_VIDEO, "Couldn't set kinect LED");
	}

	while (freenect_start_video(g_kinect_device)!=0)
	{
		SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "Couldn't start video streaming");
	}

	freenect_set_flag(g_kinect_device, FREENECT_MIRROR_VIDEO, FREENECT_ON);

	while (freenect_start_depth(g_kinect_device)!=0)
	{
		SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "Couldn't start depth streaming");
	}

	freenect_set_flag(g_kinect_device, FREENECT_MIRROR_DEPTH, FREENECT_ON);

	while (g_kinect_thread_run)
	{
		if (freenect_process_events(g_kinect_context)!=0)
		{
			SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "Couldn't process USB events");
		}
	}

	while (freenect_stop_depth(g_kinect_device)!=0)
	{
		SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "Couldn't stop depth streaming");
	}

	while (freenect_stop_video(g_kinect_device)!=0)
	{
		SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "Couldn't stop video streaming");
	}

	while (freenect_set_led(g_kinect_device, LED_RED)!=0)
	{
		SDL_LogWarn(SDL_LOG_CATEGORY_VIDEO, "Couldn't set kinect LED");
	}
}

static void kinect_video_callback(freenect_device *device, void *buffer, uint32_t timestamp)
{
	g_frame_mutex.lock();
	memcpy(&g_video_frame, buffer, sizeof(g_video_frame));
	g_frame_count++;
	g_frame_mutex.unlock();
	SDL_LogInfo(SDL_LOG_CATEGORY_VIDEO, "Received video frame at timestamp: %u", timestamp);
}

static void kinect_depth_callback(freenect_device *device, void *buffer, uint32_t timestamp)
{
	g_frame_mutex.lock();
	memcpy(&g_depth_frame, buffer, sizeof(g_depth_frame));
	g_frame_mutex.unlock();
	SDL_LogInfo(SDL_LOG_CATEGORY_VIDEO, "Received depth frame at timestamp: %u", timestamp);
}

static void camera_process_frame(
	cv::Mat3b &video_frame,
	cv::Mat1w &depth_frame,
	cv::Mat1b &edge_frame)
{
	cv::Mat gray_frame, blurred_frame;

	// convert to grayscale 
	cv::cvtColor(video_frame(cv::Rect(k_edge_x, k_edge_y, k_edge_width, k_edge_height)), gray_frame, cv::COLOR_BGR2GRAY);

	// detect edges using Canny algorithm
	cv::Canny(gray_frame, blurred_frame, 100, 200); // last two parameters are low threshold and high threshold

	// thicken the resulting edges
	cv::GaussianBlur(blurred_frame, edge_frame, cv::Size(3, 3), 2); // last two parameters are window and sigma

	// remove edges that are past the depth threshold
	cv::Mat1w clipped_depth_frame = depth_frame(cv::Rect(k_edge_x, k_edge_y, k_edge_width, k_edge_height));
	for (int i= 0; i<clipped_depth_frame.rows; i++)
	{
		for (int j= 0; j<clipped_depth_frame.cols; j++)
		{
			if (clipped_depth_frame(i, j)>=k_depth_threshold)
			{
				edge_frame(i, j)= 0;
			}
		}
	}
}
