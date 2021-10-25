#ifndef swarm_hpp
#define swarm_hpp

#include <opencv2/core.hpp>
#include <SDL.h>

#include "gesture.hpp"

const int k_simulation_width= 1080;
const int k_simulation_height= 1920;

const int k_bee_count= 10000;
const float k_bee_radius= 4.0f;

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

	void update(const cv::Mat1b &edge_frame, cv::Mat1b &field, commands_t command);
	void palm_update(command_t command,int center_x, int center_y);

	state_t state;
	float timer;
	float x, y;
	float facing;
	float speed;
	float spin;
	float last_facing;

	//rects to keep track of render frame for diff states
	SDL_Rect b_fly_rect;
	SDL_Rect b_crawl_rect;
	SDL_Rect b_idle_rect;
};

class swarm_t
{
public:
	swarm_t();
	~swarm_t();

	const bee_t *get_bees() const;
	void update(const cv::Mat1b &edge_frame, const commands_t &commands);

private:
	bee_t *bees;
	cv::Mat1b field;
};

#endif /* swarm_hpp */
