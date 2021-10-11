#ifndef swarm_h
#define swarm_h

#include <opencv2/core.hpp>

const int k_simulation_width= 1080;
const int k_simulation_height= 1920;

const int k_bee_count= 1000;

class bee_t
{
public:
	enum state_t
	{
		_idle,
		_crawling,
		_flying,
		k_state_count
	};

	bee_t();

	void update(const cv::Mat1b *edge_frame);

	state_t state;
	float timer;
	float x, y;
	float facing;
	float speed;
	float rotation;
};

class swarm_t
{
public:
	swarm_t();
	~swarm_t();

	void update(const cv::Mat1b *edge_frame);

	bee_t *bees;
};

#endif /* swarm_h */
