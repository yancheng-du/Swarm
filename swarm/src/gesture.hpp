#ifndef gesture_hpp
#define gesture_hpp

#include <string>
#include <vector>

struct command_t
{
	std::string name;
	cv::Rect bounding_box;
	float confidence;
	// $TODO add information
};

typedef std::vector<command_t> commands_t;

bool gesture_initialize();
void gesture_dispose();

bool gesture_consume_commands(commands_t &commands);

#endif /* gesture_hpp */
