#include <cmath>
#include <cstdlib>

#include "constants.hpp"
#include "swarm.hpp"

const float k_tau= 6.2831853f;

const float k_timer_minimum= 0.2f;
const float k_timer_maximum= 0.8f;

const float k_walk_speed_minimum= 8.0f;
const float k_walk_speed_maximum= 12.0f;

const float k_fly_speed_minimum= 200.0f;
const float k_fly_speed_maximum= 300.0f;

const float k_spin_maximum= 0.5f*k_tau;

static int last_draw_x= -1;
static int last_draw_y= -1;

static int last_count= 0;

static int edge_force_radius= 25;

inline float uniform_random(float minimum, float maximum)
{
	double fraction= static_cast<double>(rand())/RAND_MAX;
	return minimum+(maximum-minimum)*static_cast<float>(fraction);
}

inline float wrap_value(float value, float maximum, float gutter)
{
	while (value<-gutter) value+= maximum + 2.0f*gutter;
	while (value>=maximum+gutter) value-= maximum + 2.0f*gutter;
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
	speed= 0.0f;
	spin= 0.0f;
}

void bee_t::update(const cv::Mat1b &edge_frame, int field_max, cv::Mat1b &field)
{
	float fraction_y= y/k_simulation_height;
	float fraction_x= x/k_simulation_width;
	int edge_y= static_cast<int>(fraction_y*edge_frame.rows);
	int edge_x= static_cast<int>(fraction_x*edge_frame.cols);
	int field_y= static_cast<int>(fraction_y*field.rows);
	int field_x= static_cast<int>(fraction_x*field.cols);

	// update state, speed, and spin
	if (x>=0 && x<k_simulation_width &&
		y>=0 && y<k_simulation_height &&
		edge_frame(edge_y, edge_x)>0 &&
		field(field_y, field_x)<field_max)
	{
		field(field_y, field_x)+= 1;

		if (state==_flying || (state==_crawling && timer<0.0f))
		{
			state= _idle;
			timer= uniform_random(k_timer_minimum, k_timer_maximum);
			speed= 0.0f;
			spin= 0.0f;
		}
		else if (state==_idle && timer<0.0f)
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
	}

	// update position and facing
	x= wrap_value(x+speed*cos(facing)*k_dt, k_simulation_width, k_bee_radius);
	y= wrap_value(y+speed*sin(facing)*k_dt, k_simulation_height, k_bee_radius);
	facing= wrap_value(facing+spin*k_dt, k_tau, 0.0f);
}

void bee_t::draw_update(const cv::Mat1f &edge_frame, int field_max, cv::Mat1b &field, const cv::Mat2f &force)
{
	float fraction_y= y/k_simulation_height;
	float fraction_x= x/k_simulation_width;
	int edge_y= static_cast<int>(fraction_y*edge_frame.rows);
	int edge_x= static_cast<int>(fraction_x*edge_frame.cols);
	int field_y= static_cast<int>(fraction_y*field.rows);
	int field_x= static_cast<int>(fraction_x*field.cols);
	bool edge_move= false;
	// update state, speed, and rotation
	// if on edge
	if (x>=0.0f && x<k_simulation_width &&
		y>=0.0f && y<k_simulation_height &&
		edge_frame(edge_y, edge_x)>0.01f)
	{	//on edge and also no crowd on the same edge
		if (field(field_y, field_x)<field_max)
		{
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
		//on edge but its crowded so fly
		else
		{
			state= _flying;
			timer= uniform_random(k_timer_minimum, k_timer_maximum);
			speed= uniform_random(k_fly_speed_minimum, k_fly_speed_maximum);
		}
		field(field_y, field_x)+= 1;
	}
	//not on edge
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
		{//not on edge and is still flying	
			int i= static_cast<int>(y/k_simulation_height*force.rows);
			int j= static_cast<int>(x/k_simulation_width*force.cols);

			if (i>0 && i<force.rows && j>0 && j<force.cols)
			{
				float dx= force(i, j)[1];
				float dy= force(i, j)[0];
				if (dx!=0 || dy!=0)
				{
					spin= atan2(dy, dx);
					edge_move= true;
				}
			}
			timer-= k_dt;
		}
	}

	// update position and facing
	x= wrap_value(x+speed*cos(facing)*k_dt, k_simulation_width, k_bee_radius);
	y= wrap_value(y+speed*sin(facing)*k_dt, k_simulation_height, k_bee_radius);
	if (edge_move)
	{
		facing= wrap_value(0.2f*facing+spin, k_tau, 0.0f);
	}
	else
	{
		facing= wrap_value(facing+spin*k_dt, k_tau, 0.0f);
	}
}

void bee_t::palm_update(float center_x, float center_y, float radius)
{
	state= _flying;
	timer= 0.0f;

	if ((center_x-x)*(center_x-x) + (center_y-y)*(center_y-y)>radius*radius)
	{
		speed= uniform_random(k_fly_speed_minimum, k_fly_speed_maximum);
		float dr= atan2(center_y-y, center_x-x);
		spin= (dr-facing)/k_dt + uniform_random(0, k_spin_maximum);
	}
	else
	{
		speed= uniform_random(k_fly_speed_minimum, k_fly_speed_maximum);
		float dr=  uniform_random(-0.4*k_spin_maximum, 0.4*k_spin_maximum);
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
	//field is for bee avoid landing on an edge that already occupied by another bee
	field= cv::Mat::zeros(k_edge_height, k_edge_width, CV_8U);
	//canvas is the board for drawing
	canvas= cv::Mat::zeros(k_edge_height, k_edge_width, CV_32F);
	force= cv::Mat::zeros(k_edge_height, k_edge_width, CV_32FC2);
	init_force(edge_force_radius);
}

swarm_t::~swarm_t()
{
	delete bees;
}

void swarm_t::init_force(int edge_force_radius)
{	
	int edge_diameter= 2*edge_force_radius+1;
	//two filters that are used to calculate attraction/repulsion
	edge_attract = cv::Mat::zeros(edge_diameter, edge_diameter, CV_32FC2);
	for (int y=0; y<edge_diameter; y++)
	{
		for (int x=0; x<edge_diameter; x++)
		{
			float norm= sqrt(static_cast<float>((edge_force_radius-y)*(edge_force_radius-y) + (edge_force_radius-x)*(edge_force_radius-x)));
			if (norm==0.0f)
			{
				edge_attract(y, x)[0]= 0.0f; edge_attract(y, x)[1]= 0.0f;
			}
			else
			{
				edge_attract(y, x)[0]= (edge_force_radius-y)/norm;
				edge_attract(y, x)[1]= (edge_force_radius-x)/norm;
			}
		}
	}
}

void swarm_t::apply_filter(const cv::Mat2f &filter, int y, int x, int radius)
{
	using namespace std;
	y= min(max(0, y), force.rows);
	x= min(max(0, x), force.cols);
	int y_lower_limit= min(radius, y);						//how much can y decrease until out of bound or radius of the filter
	int y_upper_limit= min(force.rows-y, radius);			//how much can y increase until out of bound or radius of the filter
	int x_lower_limit= min(radius, x);						//how much can x decrease until out of bound or radius of the filter
	int x_upper_limit= min(force.cols-x, radius);			//how much can x increase until out of bound or radius of the filter
	for (int j= -y_lower_limit; j<y_upper_limit; j++)
	{
		for (int i= -x_lower_limit; i<x_upper_limit; i++)
		{
			force(y+j, x+i)[0]+= filter(radius+j, radius+i)[0];
			force(y+j, x+i)[1]+= filter(radius+j, radius+i)[1];
		}
	}
}

void swarm_t::get_dir_mat_float(const cv::Mat1f &edge_frame, const cv::Mat2f &edge_attract, const cv::Mat1b &field)
{
	for (int j=0; j<edge_frame.rows; j+=2)
	{
		for (int i=0; i<edge_frame.cols; i+=2)
		{
			if (edge_frame(j, i)>0.01f && field(j, i)==0)
			{
				apply_filter(edge_attract, j, i, edge_force_radius);
			}
		}
	}
}

void swarm_t::update(const cv::Mat1b &edge_frame, const commands_t &commands)
{
	bool gesture_driven= false;

	t+= k_dt;
	canvas*=0.99f;
	int line_count= count_lines(canvas);
	if (line_count<last_count) {
		last_draw_x=-1;
		last_draw_y=-1;
	}
	last_count= line_count;

	// compute field_max
	{
		int edge_count= cv::countNonZero(edge_frame);
		int field_count= edge_count*field.rows*field.cols/(edge_frame.rows*edge_frame.cols);
		field_max= 1 + (field_count>0 ? k_bee_count/field_count : 0);
		if (field_max>UINT8_MAX) field_max= UINT8_MAX;
	}

	if (commands.size()>0)
	{
		command_t current_sign= commands.at(0);

		if (current_sign.name=="palm")
		{
			float center_x= (current_sign.bounding_box.x + 0.5f*current_sign.bounding_box.width)/edge_frame.cols*k_simulation_width;
			float center_y= (current_sign.bounding_box.y + 0.5f*current_sign.bounding_box.height)/edge_frame.rows*k_simulation_height;

			field.setTo(0);

			for (int i= 0; i<k_bee_count; i++)
			{
				bees[i].palm_update(center_x, center_y, static_cast<float>(current_sign.bounding_box.width));
			}

			gesture_driven= true;
		}
		else if (current_sign.name=="peace")
		{	
			int center_x= (current_sign.bounding_box.x + current_sign.bounding_box.width/2);
			int center_y= (current_sign.bounding_box.y + current_sign.bounding_box.height/2);

			draw_line(center_x, center_y);
			force.setTo(0);
			get_dir_mat_float(canvas, edge_attract, field);
			field.setTo(0);

			for (int i= 0; i<k_bee_count; i++)
			{
				bees[i].draw_update(canvas, 500, field, force);
			}

			gesture_driven= true;
		}
	}

	if (!gesture_driven)
	{
		field.setTo(0);

		for (int i= 0; i<k_bee_count; i++)
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
	if (last_draw_x==-1 && last_draw_y==-1)
	{
		canvas(y, x)= 1.0f;
		last_draw_x= x;
		last_draw_y= y;
	}
	//not the first point
	else
	{	//calculate slopt and intercept
		float m= x==last_draw_x ? 0.0f : static_cast<float>(y-last_draw_y)/(x-last_draw_x);
		float b= y-m*x;
		//fill points on the canvas
		if (x>last_draw_x)
		{
			for (int i= last_draw_x; i<=x; i++)
			{
				canvas(static_cast<int>(m*i+b), i)= 1.0f;
			}
		}
		else
		{
			for (int i= x; i<=last_draw_x; i++)
			{
				canvas(static_cast<int>(m*i+b), i)= 1.0f;
			}
		}
		last_draw_x= x;
		last_draw_y= y;
	}
}

int swarm_t::count_lines(const cv::Mat1f &canvas)
{
	int count=0;
	for (int j= 0; j<canvas.rows; j++) 
	{
		for (int i= 0; i<canvas.cols; i++)
		{
			if (canvas(j, i)>0.01f) 
			{
				count++;
			}
		}
	}
	return count;
}
