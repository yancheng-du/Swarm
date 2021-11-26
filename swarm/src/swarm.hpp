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

	void update(const cv::Mat1b &edge_frame, int landed_max, cv::Mat1b &landed);
	void palm_update(float center_x, float center_y, float radius);
	void draw_update(const cv::Mat1f &edge_frame, int landed_max, cv::Mat1b &landed, const cv::Mat2f &force);

	state_t state;
	float timer;
	float x, y;
	float facing;
	float speed;
	float spin;
};

class swarm_t
{
public:
	swarm_t();
	~swarm_t();

	void reset();

	void update(const cv::Mat1b &edge_frame, const commands_t &commands);
	void draw_line(int x, int y);
	int count_lines(const cv::Mat1f &canvas);

	void get_dir_mat_float(const cv::Mat1f &edge_frame, const cv::Mat2f &edge_attract, const cv::Mat1b &landed);
	void apply_filter(const cv::Mat2f &force_field, int x, int y, int radius);
	void init_force(int edge_force_size);

	double t;
	bee_t *bees;
	float state_fractions[bee_t::k_state_count]; // fraction of total bees in each state

	int landed_max;
	cv::Mat1b landed;
	cv::Mat1f canvas;

	cv::Mat2f force;
	cv::Mat2f edge_attract;
};

#endif /* swarm_hpp */
