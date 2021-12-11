#include <cmath>
#include <cstdlib>
#include <queue>

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

const int k_field_width= k_simulation_width/24;
const int k_field_height= k_simulation_height/24;

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

void bee_t::update(const cv::Mat1b &edge_frame, int landed_max, cv::Mat1b &landed, const cv::Mat1f *flow)
{
	float fraction_y= y/k_simulation_height;
	float fraction_x= x/k_simulation_width;
	int edge_y= static_cast<int>(fraction_y*edge_frame.rows);
	int edge_x= static_cast<int>(fraction_x*edge_frame.cols);
	int landed_y= static_cast<int>(fraction_y*landed.rows);
	int landed_x= static_cast<int>(fraction_x*landed.cols);

	// update state, speed, and spin
	if (x>=0 && x<k_simulation_width &&
		y>=0 && y<k_simulation_height &&
		edge_frame(edge_y, edge_x)>0 &&
		landed(landed_y, landed_x)<landed_max)
	{
		landed(landed_y, landed_x)+= 1;

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
			if (flow && 
				x>=0 && x<k_simulation_width &&
				y>=0 && y<k_simulation_height)
			{
				int flow_y= static_cast<int>(fraction_y*flow->rows);
				int flow_x= static_cast<int>(fraction_x*flow->cols);
				float desired_facing= (*flow)(flow_y, flow_x);
				float delta= desired_facing - facing;

				if (delta>0.5*k_tau) delta-= k_tau;
				else if (delta<-0.5*k_tau) delta+= k_tau;

				spin= delta>0 ? uniform_random(0.0f, k_spin_maximum) : uniform_random(-k_spin_maximum, 0.0f);
			}
			else
			{
				spin= uniform_random(-k_spin_maximum, k_spin_maximum);
			}
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

void bee_t::draw_update(const cv::Mat1f &edge_frame, int landed_max, cv::Mat1b &landed, const cv::Mat2f &force)
{
	float fraction_y= y/k_simulation_height;
	float fraction_x= x/k_simulation_width;
	int edge_y= static_cast<int>(fraction_y*edge_frame.rows);
	int edge_x= static_cast<int>(fraction_x*edge_frame.cols);
	int landed_y= static_cast<int>(fraction_y*landed.rows);
	int landed_x= static_cast<int>(fraction_x*landed.cols);
	bool edge_move= false;
	// update state, speed, and rotation
	// if on edge
	if (x>=0.0f && x<k_simulation_width &&
		y>=0.0f && y<k_simulation_height &&
		edge_frame(edge_y, edge_x)>0.01f)
	{	//on edge and also no crowd on the same edge
		if (landed(landed_y, landed_x)<landed_max)
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
		landed(landed_y, landed_x)+= 1;
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

swarm_t::swarm_t(): bees(NULL)
{
	reset();
}

swarm_t::~swarm_t()
{
	delete bees;
}

void swarm_t::reset()
{
	if (bees) delete bees;

	bees= new bee_t[k_bee_count];

	landed_max= 0;
	landed= cv::Mat::zeros(k_field_height, k_field_width, CV_8U);
	flow_active= true;
	flow= cv::Mat::zeros(k_field_height, k_field_width, CV_32F);
	canvas= cv::Mat::zeros(k_edge_height, k_edge_width, CV_32F);
	force= cv::Mat::zeros(k_edge_height, k_edge_width, CV_32FC2);
	init_force(edge_force_radius);
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

void swarm_t::get_dir_mat_float(const cv::Mat1f &edge_frame, const cv::Mat2f &edge_attract, const cv::Mat1b &landed)
{
	for (int j=0; j<edge_frame.rows; j+=2)
	{
		int landed_j= j*landed.rows/edge_frame.rows;

		for (int i=0; i<edge_frame.cols; i+=2)
		{
			int landed_i= i*landed.cols/edge_frame.cols;

			if (edge_frame(j, i)>0.01f && landed(landed_j, landed_i)==0)
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

	// compute landed_max
	{
		int edge_count= cv::countNonZero(edge_frame);
		int landed_count= edge_count*landed.rows*landed.cols/(edge_frame.rows*edge_frame.cols);
		landed_max= (landed_count>0 ? k_bee_count/landed_count : 0);
		if (landed_max<=0) landed_max= 1;
		else if (landed_max>UINT8_MAX) landed_max= UINT8_MAX;
	}

	if (commands.size()>0)
	{
		command_t current_sign= commands.at(0);

		if (current_sign.name=="palm")
		{
			float center_x= (current_sign.bounding_box.x + 0.5f*current_sign.bounding_box.width)/edge_frame.cols*k_simulation_width;
			float center_y= (current_sign.bounding_box.y + 0.5f*current_sign.bounding_box.height)/edge_frame.rows*k_simulation_height;

			landed.setTo(0);

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
			get_dir_mat_float(canvas, edge_attract, landed);
			landed.setTo(0);

			for (int i= 0; i<k_bee_count; i++)
			{
				bees[i].draw_update(canvas, 500, landed, force);
			}

			gesture_driven= true;
		}
	}

	if (!gesture_driven)
	{
		landed.setTo(0);

		for (int i= 0; i<k_bee_count; i++)
		{	
			bees[i].update(edge_frame, landed_max, landed, flow_active? &flow : NULL);
		}
	}

	// compute flow for next update
	if (flow_active)
	{
		struct point_t
		{
			int8_t x, y;

			point_t(): x(0), y(0) {}
			point_t(int i, int j): x(i), y(j) {}
		};

		struct element_t
		{
			uint16_t distance; // squared
			point_t point;

			element_t(): distance(0) {};
			element_t(int x, int y, const point_t &np): point(x, y), distance((x-np.x)*(x-np.x)+(y-np.y)*(y-np.y)) {}
			bool operator()(const element_t &a, const element_t &b) { return a.distance>b.distance; }
		};

		point_t nearest_uncovered_edge[k_field_height][k_field_width];
		std::priority_queue<element_t, std::vector<element_t>, element_t> queue;

		memset(nearest_uncovered_edge, -1, sizeof(nearest_uncovered_edge));

		// make an uncovered edge map
		assert(landed.rows==k_field_height);
		assert(landed.cols==k_field_width);
		{
			int edge_dx= edge_frame.cols/landed.cols;
			int edge_dy= edge_frame.rows/landed.rows;

			for (int y= 0; y<landed.rows; y++)
			{
				for (int x= 0; x<landed.cols; x++)
				{
					int landed_count= landed(y, x);
					int edge_count= cv::countNonZero(edge_frame(cv::Rect(x*edge_dx, y*edge_dy, edge_dx, edge_dy)));

					if (landed_count*edge_dx*edge_dy<edge_count*landed_max/2)
					{
						nearest_uncovered_edge[y][x]= point_t(x, y);
						queue.push(element_t(x, y, nearest_uncovered_edge[y][x]));
					}
				}
			}
		}

		// flood fill from uncovered edges to find closest uncovered edge
		if (!queue.empty())
		{
			while (!queue.empty())
			{
				const int count= 4;
				const int dx[count]={1, 0, -1, 0};
				const int dy[count]={0, 1, 0, -1};

				element_t element= queue.top();

				queue.pop();

				for (int index= 0; index<count; index++)
				{
					int x= element.point.x + dx[index];
					int y= element.point.y + dy[index];

					if (x>=0 && x<k_field_width &&
						y>=0 && y<k_field_height &&
						nearest_uncovered_edge[y][x].x==-1 &&
						nearest_uncovered_edge[y][x].y==-1)
					{
						nearest_uncovered_edge[y][x]= nearest_uncovered_edge[element.point.y][element.point.x];
						queue.push(element_t(x, y, nearest_uncovered_edge[element.point.y][element.point.x]));
					}
				}
			}

			// compute flow from closest uncovered edge
			assert(flow.rows==k_field_height);
			assert(flow.cols==k_field_width);
			{
				for (int y= 0; y<flow.rows; y++)
				{
					for (int x= 0; x<flow.cols; x++)
					{
						float dx= static_cast<float>(nearest_uncovered_edge[y][x].x - x);
						float dy= static_cast<float>(nearest_uncovered_edge[y][x].y - y);

						if (dx!=0.0f || dy!=0.0f)
						{
							flow(y, x)= atan2(dy, dx);
						}
					}
				}
			}
		}
		else
		{
			int x_mid= flow.cols/2;
			int y_mid= flow.rows/2;

			for (int y= 0; y<flow.rows; y++)
			{
				for (int x= 0; x<flow.cols; x++)
				{
					float dx= static_cast<float>(x - x_mid);
					float dy= static_cast<float>(y - y_mid);

					if (dx!=0.0f || dy!=0.0f)
					{
						flow(y, x)= atan2(dy, dx);
					}
				}
			}
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
	else if (x!=last_draw_x || y!=last_draw_y)
	{
		float dx= static_cast<float>(x-last_draw_x);
		float dy= static_cast<float>(y-last_draw_y);

		if (std::abs(dx)>std::abs(dy))
		{
			//calculate slope and y-intercept
			float m= dy/dx;
			float b= y - m*x;
			int x_min= dx>0 ? last_draw_x : x;
			int x_max= dx>0 ? x : last_draw_x;

			//iterate x, fill points on the canvas
			for (int i= x_min; i<=x_max; i++)
			{
				canvas(static_cast<int>(m*i+b), i)= 1.0f;
			}
		}
		else
		{
			//calculate inverse slope and x-intercept
			float m= dx/dy;
			float b= x - m*y;
			int y_min= dy>0 ? last_draw_y : y;
			int y_max= dy>0 ? y : last_draw_y;

			//iterate y, fill points on the canvas
			for (int j= y_min; j<=y_max; j++)
			{
				canvas(j, static_cast<int>(m*j+b))= 1.0f;
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
