#ifndef swarm_h
#define swarm_h

#include <opencv2/core.hpp>

class bee_t
{
public:
	bee_t();

	void update(const cv::Mat1b *edge_frame, float dt);

	int p_x, p_y;
	int v_x, v_y;
};

class swarm_t
{
public:
	swarm_t();
	~swarm_t();

	void update(const cv::Mat1b *edge_frame, float dt);

	bee_t *bees;
};

#endif /* swarm_h */
