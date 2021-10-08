#include <cstdlib>
#include <opencv2/core.hpp>

#include "swarm.h"

const int k_bee_velocity= 15;

bee_t::bee_t()
{
	p_x= rand()%k_simulation_width;
	p_y= rand()%k_simulation_height;
	v_x= rand()%k_bee_velocity - k_bee_velocity/2;
	v_y= rand()%k_bee_velocity - k_bee_velocity/2;
}

void bee_t::update(const cv::Mat1b *edge_frame, float dt)
{
	if (edge_frame->at<bool>(p_y*edge_frame->rows/k_simulation_height, p_x*edge_frame->cols/k_simulation_width)!=0)
	{
		v_x= 0;
		v_y= 0;
	}
	else
	{
		//bees have an 2% chance to
		if (rand()%100<2)
		{
			v_x+= rand()%k_bee_velocity - k_bee_velocity/2;
			v_y+= rand()%k_bee_velocity - k_bee_velocity/2;
		}
	}

	if (p_x+v_x<0 || p_x+v_x>=k_simulation_width)
	{
		v_x= -v_x;
	}

	if (p_y+v_y<0 || p_y+v_y>=k_simulation_height)
	{
		v_y= -v_y;
	}

	p_x+= v_x;
	p_y+= v_y;
}

swarm_t::swarm_t()
{
	bees= new bee_t[k_bee_count];
}

swarm_t::~swarm_t()
{
	delete(bees);
}

void swarm_t::update(const cv::Mat1b *edge_frame, float dt)
{
	for (int i= 0; i<k_bee_count; i++)
	{
		bees[i].update(edge_frame, dt);
	}
}
