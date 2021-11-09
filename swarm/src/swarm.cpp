#include <cmath>
#include <cstdlib>

#include "constants.hpp"
#include "swarm.hpp"

const float k_tau= 6.2831853f;

const float k_timer_minimum= 0.2f;
const float k_timer_maximum= 0.8f;

// $TODO implement walking in direction of edges
const float k_walk_speed_minimum= 8.0f;
const float k_walk_speed_maximum= 12.0f;

const float k_fly_speed_minimum= 200.0f;
const float k_fly_speed_maximum= 300.0f;

const float k_spin_maximum= 0.5f*k_tau;

static int last_draw_x= -1;
static int last_draw_y= -1;

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
	spin= 0.0f;
}

void bee_t::update(const cv::Mat1b &edge_frame, int field_max, cv::Mat1b &field)
{	
	int field_y= static_cast<int>(y/k_simulation_height*field.rows);
	int field_x= static_cast<int>(x/k_simulation_width*field.cols);

	// update state, speed, and rotation
	if (x>=0.0f && x<k_simulation_width &&
		y>=0.0f && y<k_simulation_height &&
		edge_frame(static_cast<int>(y/k_simulation_height*edge_frame.rows), static_cast<int>(x/k_simulation_width*edge_frame.cols))>0 &&
		field(field_y, field_x)<field_max)
	{
		field(field_y, field_x)+= 1;

		if (state==_flying || (state==_crawling&&timer<0.0f))
		{
			state= _idle;
			timer= uniform_random(k_timer_minimum, k_timer_maximum);
			speed= 0.0f;
			spin= 0.0f;
		}
		else if (state==_idle&&timer<0.0f)
		{
			state= _crawling;
			timer= uniform_random(k_timer_minimum, k_timer_maximum);
			speed= uniform_random(k_walk_speed_minimum, k_walk_speed_maximum);
			spin= uniform_random(-k_spin_maximum, k_spin_maximum);
		}
		else
		{
			timer-= k_dt;
		}
	}
	else
	{
		if (state!=_flying || timer<0.0f)
		{
			state= _flying;
			timer= uniform_random(k_timer_minimum, k_timer_maximum);
			speed= uniform_random(k_fly_speed_minimum, k_fly_speed_maximum);
			spin= uniform_random(-k_spin_maximum, k_spin_maximum);
		}
		else
		{
			timer-= k_dt;
		}

		// $TODO fly towards edges if near
	}

	// update position and facing
	x= wrap_value(x+speed*cos(facing)*k_dt, k_simulation_width, k_bee_radius);
	y= wrap_value(y+speed*sin(facing)*k_dt, k_simulation_height, k_bee_radius);
	facing= wrap_value(facing+spin*k_dt, k_tau, 0.0f);
}

void bee_t::palm_update(int bound_width, int center_x, int center_y)
{
	if ((center_x-x)*(center_x-x) + (center_y-y)*(center_y-y)>bound_width*bound_width)
	{
		speed= uniform_random(k_fly_speed_minimum, k_fly_speed_maximum);
		double dr= atan2(center_y-y, center_x-x) + uniform_random(0, 0.4*k_spin_maximum);
		spin= (dr-facing)/k_dt;
	}
	else
	{
		speed= uniform_random(k_fly_speed_minimum, k_fly_speed_maximum);
		double dr=  uniform_random(-0.4*k_spin_maximum, 0.4*k_spin_maximum);
		spin= dr;
	}

	// update position and facing
	x= wrap_value(x+speed*cos(facing)*k_dt, k_simulation_width, k_bee_radius);
	y= wrap_value(y+speed*sin(facing)*k_dt, k_simulation_height, k_bee_radius);
	facing= wrap_value(facing+spin*k_dt, k_tau, 0.0f);
}

swarm_t::swarm_t()
{
	bees= new bee_t[k_bee_count];
	field= cv::Mat::zeros(k_simulation_height/static_cast<int>(k_bee_radius), k_simulation_width/static_cast<int>(k_bee_radius), CV_8U);
	canvas = cv::Mat::zeros(k_edge_height, k_edge_width,CV_8U);
}

swarm_t::~swarm_t()
{
	delete bees;
}

void swarm_t::update(const cv::Mat1b &edge_frame, const commands_t &commands)
{
	t+= k_dt;
	field.setTo(0);

	// compute field_max
	{
		int edge_count= 0;

		for (int edge_y= 0; edge_y<edge_frame.rows; edge_y++)
		{
			for (int edge_x= 0; edge_x<edge_frame.cols; edge_x++)
			{
				if (edge_frame(edge_y, edge_x)>0)
				{
					edge_count+= 1;
				}
			}
		}

		int field_count= edge_count*field.rows*field.cols/(edge_frame.rows*edge_frame.cols);
		field_max= 1 + (field_count>0 ? k_bee_count/field_count : 0);
		if (field_max>UINT8_MAX) field_max= UINT8_MAX;
	}

	if (commands.size()>0)
	{
		command_t current_sign = commands.at(0);
		//currently only have palm interaction
		if (current_sign.name == "palm")
		{
			int center_x, center_y;
			center_x= (current_sign.bounding_box.x + 0.5*current_sign.bounding_box.width)/edge_frame.cols*k_simulation_width;
			center_y= (current_sign.bounding_box.y + 0.5*current_sign.bounding_box.height)/edge_frame.rows*k_simulation_height;
			for (int i = 0; i<k_bee_count; i++)
			{
				bees[i].palm_update(current_sign.bounding_box.width, center_x, center_y);
			}
		}
		else if (current_sign.name == "peace")
		{
			int center_x, center_y;
			center_x= (current_sign.bounding_box.x + 0.5*current_sign.bounding_box.width);
			center_y= (current_sign.bounding_box.y + 0.5*current_sign.bounding_box.height);
			draw_line(center_x, center_y);
			for (int i = 0; i<k_bee_count; i++)
			{
				bees[i].update(canvas, field_max, field);
			}
		}
		//all other gesture will be treated as normal bee update
		else 
		{
			for (int i = 0; i<k_bee_count; i++)
			{
				bees[i].update(edge_frame, field_max, field);
			}
		}
	}
	else
	{	
		//no gestures
		for (int i = 0; i<k_bee_count; i++)
		{
			bees[i].update(edge_frame, field_max, field);
		}
	}

	// update state fractions
	{
		int state_counts[bee_t::k_state_count];

		for (int state= 0; state<bee_t::k_state_count; state++)
		{
			state_counts[state]= 0;
		}

		for (int bee_index= 0; bee_index<k_bee_count; bee_index++)
		{
			state_counts[bees[bee_index].state]+= 1;
		}

		for (int state= 0; state<bee_t::k_state_count; state++)
		{
			state_fractions[state]= static_cast<float>(state_counts[state])/k_bee_count;
		}
	}
}

void swarm_t::draw_line(int x, int y)
{
	//if it is the first point
	if (last_draw_x ==-1 && last_draw_y==-1)
	{
		canvas.at<bool>(y, x) = 1;
		last_draw_x = x;
		last_draw_y = y;
	}
	//not the first point
	else
	{	//calculate slopt and intercept
		float m;
		if (x==last_draw_x)
		{
			m=0;
		}
		else
		{
			m = (float)(y - last_draw_y)/(x-last_draw_x);
		}
		float b = y-m*x;
		//fill points on the canvas
		if (x > last_draw_x)
		{
			for (int i = last_draw_x; i<=x; i++)
			{
				canvas.at<bool>((int)(m*i+b), i) = 1;
			}
		}
		else
		{
			for (int i = x; i<=last_draw_x; i++)
			{
				canvas.at<bool>((int)(m*i+b), i) = 1;
			}
		}
		last_draw_x = x;
		last_draw_y = y;
	}

}
