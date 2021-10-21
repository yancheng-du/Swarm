#include <mutex>
#include <thread>

#include "camera.hpp"
#include "constants.hpp"
#include "gesture.hpp"
#include "model.hpp"

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
	commands_t commands;
	model_t model;

	while (g_gesture_thread_run)
	{
		camera_peek_video_frame(video_frame);
		model.analyze_frame(video_frame, commands);

		g_commands_mutex.lock();
		g_commands= commands;
		g_commands_available= true;
		g_commands_mutex.unlock();
	}
}
