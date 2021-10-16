#ifndef swarm_hpp
#define swarm_hpp

#include <opencv2/core.hpp>

const int k_simulation_width = 1080;
const int k_simulation_height = 1920;

const int k_bee_count = 20000;

class bee_t
{
public:
	enum state_t
	{
		_idle,
		_crawling,
		_flying,
		_accelerating,
		k_state_count
	};

	bee_t();

	void update(const cv::Mat1b* edge_frame);

	state_t state;
	float timer;
	float x, y;
	float facing;
	float speed;
	float rotation;
	float last_facing;
};

class swarm_t
{
public:
	swarm_t();
	~swarm_t();

	void update(const cv::Mat1b* edge_frame);

	bee_t* bees;
};

#endif /* swarm_hpp */