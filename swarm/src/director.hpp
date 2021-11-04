#ifndef director_hpp
#define director_hpp

#include <opencv2/core.hpp>

#include "gesture.hpp"
#include "swarm.hpp"

class director_t
{
public:
	director_t();
	~director_t();

	bool is_running();
	void do_frame();
	void process_events();

private:
	bool running= true;
	bool fullscreen= false;
	bool idle= false;
	bool debug= false;
	bool fps= false;

	cv::Mat3b video_frame;
	cv::Mat3b last_video_frame;
	cv::Mat1w depth_frame;
	cv::Mat1b edge_frame;
	commands_t commands;

	swarm_t swarm;
};

#endif /* director_hpp */
