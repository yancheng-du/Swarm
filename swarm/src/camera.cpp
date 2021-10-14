#include <cassert>
#include <cstring>
#include <mutex>
#include <thread>
#include <libfreenect.h>
#include <SDL_log.h>

#include <opencv2/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>

#include "camera.h"

const int k_camera_width= 640;
const int k_camera_height= 480;

const int k_edge_width= k_camera_height*9/16;

static void kinect_thread_function();
static void kinect_video_callback(freenect_device *device, void *buffer, uint32_t timestamp);
static void kinect_depth_callback(freenect_device *device, void *buffer, uint32_t timestamp);

static freenect_context *g_kinect_context= NULL;
static freenect_device *g_kinect_device= NULL;
static bool g_kinect_thread_run= false;
static std::thread *g_kinect_thread= NULL;

static std::mutex g_frame_mutex;
static uint8_t g_video_frame[k_camera_width*k_camera_height*3]= {0};
static uint16_t g_depth_frame[k_camera_width*k_camera_height]= {0};
static int g_frame_count= 0;

static int8_t* x_field;
static int8_t* y_field;


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
					init_field(23);
					#ifdef DEBUG
					freenect_frame_mode video_mode= freenect_get_current_video_mode(g_kinect_device);
					assert(video_mode.is_valid);
					assert(video_mode.video_format==FREENECT_VIDEO_RGB);
					assert(video_mode.width==k_camera_width);
					assert(video_mode.height==k_camera_height);
					assert(video_mode.bytes==sizeof(g_video_frame));
					#endif

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

						freenect_set_video_callback(g_kinect_device, kinect_video_callback);
						freenect_set_depth_callback(g_kinect_device, kinect_depth_callback);

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

int camera_read_frame(
	cv::Mat3b *video_frame,
	cv::Mat1w *depth_frame,
	cv::Mat1b *edge_frame)
{
	int frame_count;

	video_frame->create(k_camera_height, k_camera_width);
	assert(video_frame->isContinuous());
	assert(video_frame->dataend-video_frame->datastart==sizeof(g_video_frame));

	depth_frame->create(k_camera_height, k_camera_width);
	assert(depth_frame->isContinuous());
	assert(depth_frame->dataend-depth_frame->datastart==sizeof(g_depth_frame));

	g_frame_mutex.lock();
	std::memcpy(video_frame->data, g_video_frame, sizeof(g_video_frame));
	std::memcpy(depth_frame->data, g_depth_frame, sizeof(g_depth_frame));
	frame_count= g_frame_count;
	g_frame_mutex.unlock();

	cv::Mat grey_frame, blurred_frame;
	cv::cvtColor((*video_frame)(cv::Rect((k_camera_width-k_edge_width)/2, 0, k_edge_width, k_camera_height)), grey_frame, cv::COLOR_BGR2GRAY);
	cv::Canny(grey_frame, 		// input image
		*edge_frame, 				// output image
		50, 						// low threshold
		200);

	return frame_count;
}

void init_field(size_t field_size) {
	x_field = new int8_t[field_size * field_size];
	y_field = new int8_t[field_size * field_size];
	for (size_t i = 0; i < field_size; i++) {
		for (size_t j = 0; j < field_size; j++) {
			x_field[i * field_size + j] = (field_size - 1) / 2 - j;
			y_field[i * field_size + j] = (field_size - 1) / 2 - i;
		}
	}

	//for (int i = 0; i < field_size; i++) {
	//	for (int j = 0; j < field_size; j++) {
	//		printf("%d  ", y_field[i*field_size + j]);
	//	}
	//	printf("\n");
	//}
}


void get_vector_frame(
	cv::Mat1b* edge_frame,
	cv::Mat* x_vector_frame,
	cv::Mat* y_vector_frame,
	size_t field_size
) {
	for (int i = 0; i < edge_frame->rows; i++) {
		for (int j = 0; j < edge_frame->cols; j++) {
			if (edge_frame->at<bool>(i, j) != 0) {
				//printf("here\n");
				for (int vec_i = 0; vec_i < field_size; vec_i++) {
					for (int vec_j = 0; vec_j < field_size; vec_j++) {
						int i_new = i + vec_i - (field_size - 1) / 2;
						int j_new = j + vec_j - (field_size - 1) / 2;
						if (i_new >= 0 && i_new < edge_frame->rows && j_new >= 0 && j_new < edge_frame->cols) {
							//printf("getting vec from edge\n");
							x_vector_frame->at<int8_t>(i_new, j_new) = x_field[vec_i * field_size + vec_j];
							y_vector_frame->at<int8_t>(i_new, j_new) = y_field[vec_i * field_size + vec_j];
						}
					}
				}
			}
		}
	}
	//puts("finished getting vector frame");
	//for (int i = 0; i < x_vector_frame->rows; i++) {
	//	for (int j = 0; j < x_vector_frame->cols; j++) {
	//		printf("%d  ", x_vector_frame->at<int8_t>(i, j));
	//	}
	//	printf("\n");
	//}

	//puts("");
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
