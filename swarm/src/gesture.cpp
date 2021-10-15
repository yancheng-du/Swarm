#include <mutex>
#include <thread>

#include <opencv2/core.hpp>

#include "camera.h"
#include "constants.h"
#include "gesture.hpp"

static void gesture_thread_function();

static bool g_gesture_thread_run= false;
static std::thread *g_gesture_thread= NULL;

static std::mutex g_commands_mutex;
static commands_t g_commands;
static bool g_commands_available= false;

bool gesture_initialize()
{
	g_gesture_thread_run= true;
	g_gesture_thread= new std::thread(gesture_thread_function);

	return true;
}

void gesture_dispose()
{
	if (g_gesture_thread)
	{
		g_gesture_thread_run= false;
		g_gesture_thread->join();
		delete g_gesture_thread;
		g_gesture_thread= NULL;
	}
}

bool gesture_consume_commands(commands_t &commands)
{
	bool consumed= false;

	if (g_commands_available)
	{
		g_commands_mutex.lock();
		commands= g_commands;
		g_commands.clear();
		g_commands_available= false;
		g_commands_mutex.unlock();

		consumed= true;
	}

	return consumed;
}

static void gesture_thread_function()
{
	cv::Mat3b video_frame;

	// $TODO create/load model

	while (g_gesture_thread_run)
	{
		camera_peek_video_frame(&video_frame);

		// $TODO analyze video fame with model and remove the hack sleep below

		// HACK to represent model analyzing time (so that we don't busy loop copy the video frame)
		std::this_thread::sleep_for(std::chrono::milliseconds(50));

		g_commands_mutex.lock();
		g_commands.clear();
		// $TODO add commands from model (note we always set queue available because no commands is valid too)
		g_commands_available= true;
		g_commands_mutex.unlock();
	}

	// $TODO free model
}
