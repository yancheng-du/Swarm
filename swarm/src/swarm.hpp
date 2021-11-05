#ifndef swarm_hpp
#define swarm_hpp

#include <opencv2/core.hpp>

#include "gesture.hpp"

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

	void update(const cv::Mat1b &edge_frame, int field_max, cv::Mat1b &field);
	void palm_update(int bound_width,int center_x, int center_y);
	void draw_update(const cv::Mat1b& canvas);

	state_t state;
	float timer;
	float x, y;
	float facing;
	float speed;
	float spin;
	float last_facing;
};

class swarm_t
{
public:
	swarm_t();
	~swarm_t();

	void update(const cv::Mat1b &edge_frame, const commands_t &commands);
	void draw_line(int x, int y);

	double t;
	bee_t *bees;
	float state_fractions[bee_t::k_state_count]; // fraction of total bees in each state

	int field_max;
	cv::Mat1b field;
	cv::Mat1b canvas;
};

#endif /* swarm_hpp */
