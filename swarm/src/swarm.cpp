#include <cmath>
#include <cstdlib>

#include "constants.hpp"
#include "swarm.hpp"

const int b_texture_h = 64;
const int b_fly_texture_w = 1920;
const int b_crawl_texture_w = 3904;
const int b_idle_texture_w = 3904;
const int b_frame_h = 64;
const int b_frame_w = 64;
const int b_fly_frame_count = 30;
const int b_crawl_frame_count = 61;
const int b_idle_frame_count = 61;

const float k_tau= 6.2831853f;

const float k_bee_radius= 1.0f;

const float k_timer_minimum= 0.2f;
const float k_timer_maximum= 0.8f;

// $TODO implement walking in direction of edges
const float k_walk_speed_minimum= 0.0f;
const float k_walk_speed_maximum= 0.0f;

const float k_acc_minimum= 0.0f;
const float k_acc_maximum= 10.0f;

const float k_fly_speed_minimum= 120;
const float k_fly_speed_maximum= 180;

const float k_spin_maximum= 0.5f*k_tau;

inline float uniform_random(float minimum, float maximum)
{
	double fraction= static_cast<double>(rand())/RAND_MAX;
	return minimum+(maximum-minimum)*static_cast<float>(fraction);
}

inline float wrap_value(float value, float maximum, float gutter)
{
	while (value<-gutter) value += maximum+2.0f*gutter;
	while (value>=maximum+gutter) value -= maximum+2.0f*gutter;
	return value;
}

bee_t::bee_t()
{
	float edge_position= uniform_random(0.0f, k_simulation_width+k_simulation_height+4.0f*k_bee_radius);
	bool top_edge= edge_position<k_simulation_width+2.0f*k_bee_radius;

	state= _idle;
	timer= 0.0f;
	x= top_edge ? edge_position-k_bee_radius : -k_bee_radius;
	y= top_edge ? -k_bee_radius : edge_position-k_simulation_width-3.0f*k_bee_radius;
	assert(x>=-k_bee_radius && x<k_simulation_width+k_bee_radius);
	assert(y>=-k_bee_radius && y<k_simulation_height+k_bee_radius);
	facing= uniform_random(0.0f, k_tau);
	last_facing= facing;
	speed= 0.0f;
	rotation= 0.0f;

	//at init, every bee starts at random render frame
	
	b_fly_rect.x = (rand() % b_fly_frame_count) * b_frame_w;
	b_crawl_rect.x = (rand() % b_crawl_frame_count) * b_frame_w;
	b_idle_rect.x = (rand() % b_idle_frame_count) * b_frame_w;

	b_fly_rect.y = b_crawl_rect.y = b_idle_rect.y = 0;
	b_fly_rect.w = b_crawl_rect.w = b_idle_rect.w = b_frame_w;
	b_fly_rect.h = b_crawl_rect.h = b_idle_rect.h = b_frame_h;

}

void bee_t::update(const cv::Mat1b* edge_frame)
{
	// update state, speed, and rotation
	if (x>=0.0f && x<k_simulation_width &&
		y>=0.0f && y<k_simulation_height &&
		edge_frame->at<bool>(static_cast<int>(y/k_simulation_height*edge_frame->rows), static_cast<int>(x/k_simulation_width*edge_frame->cols))!=0)
	{
		if (state==_flying || (state==_crawling&&timer<0.0f) || state==_accelerating)
		{
			state= _idle;
			timer= uniform_random(k_timer_minimum, k_timer_maximum);
			speed= 0.0f;
			rotation= 0.0f;
		}
		//craw on edge
		else if (state==_idle&&timer<0.0f)
		{
			state= _crawling;
			timer= uniform_random(k_timer_minimum, k_timer_maximum);
			speed= uniform_random(k_walk_speed_minimum, k_walk_speed_maximum);
			rotation= uniform_random(-k_spin_maximum, k_spin_maximum);
		}
		else
		{
			timer-= k_dt;
		}
	}
	else
	{ //not on edge, keep flying
		if (state==_flying && timer<0.0f)
		{
			state= _flying;
			timer= uniform_random(k_timer_minimum, k_timer_maximum);
			speed= uniform_random(k_fly_speed_minimum, k_fly_speed_maximum);
			rotation= uniform_random(-k_spin_maximum, k_spin_maximum);
		}
		else
		{ //not on edge, accelerating
			if (state!=_flying)
			{
				state= _accelerating;
				if (speed>=k_fly_speed_minimum)
				{
					state= _flying;
				}
				timer= uniform_random(k_timer_minimum, k_timer_maximum);
				speed+= uniform_random(k_acc_minimum, k_acc_maximum);
				rotation= uniform_random(-k_spin_maximum, k_spin_maximum);
			}
		}
		timer-= k_dt;
		// $TODO fly towards edges if near
	}

	// update position and facing
	x= wrap_value(x+speed*cos(facing)*k_dt, k_simulation_width, k_bee_radius);
	y= wrap_value(y+speed*sin(facing)*k_dt, k_simulation_height, k_bee_radius);
	facing= wrap_value(facing+rotation*k_dt, k_tau, 0.0f);
	
	//update sprite render frame
	b_fly_rect.x += b_frame_w;
	b_crawl_rect.x += b_frame_w;
	b_idle_rect.x += b_frame_w;

	//check if past texture
	if (b_fly_rect.x >= b_fly_texture_w)
		b_fly_rect.x = 0;
	if (b_crawl_rect.x >= b_crawl_texture_w)
		b_crawl_rect.x = 0;
	if (b_idle_rect.x >= b_idle_texture_w)
		b_idle_rect.x = 0;
}

swarm_t::swarm_t()
{
	bees = new bee_t[k_bee_count];
}

swarm_t::~swarm_t()
{
	delete (bees);
}

void swarm_t::update(const cv::Mat1b* edge_frame)
{
	for (int i = 0; i<k_bee_count; i++)
	{
		bees[i].update(edge_frame);
	}
}
