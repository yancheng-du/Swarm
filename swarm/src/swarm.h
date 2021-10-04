#ifndef swarm_h
#define swarm_h

#include "camera_frame.h"

class bee_t
{
public:
	bee_t();

	void update(const edge_frame_t edge_frame, float dt);

	int p_x, p_y;
	int v_x, v_y;
};

class swarm_t
{
public:
	swarm_t();
	~swarm_t();

	void update(const edge_frame_t edge_frame, float dt);

	bee_t *bees;
};

#endif /* swarm_h */
