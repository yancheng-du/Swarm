#ifndef SWARM_HEADER
#define SWARM_HEADER

#include "SDL.h"
#include <stdio.h>      /* printf, scanf, puts, NULL */
#include <stdlib.h>     /* srand, rand */

#include "const.h"
#include "camera_frame.h"


class Bee {
public:
	Bee();
	~Bee();

	void bee_init();
	void bee_update(edge_frame_t edge_frame);
	SDL_Point get_pos() {
		return pos;
	}

private:
	int x, y;
	int v_x, v_y;
	SDL_Point pos;
};




class Swarm
{

public:
	Swarm();
	~Swarm();

	void swarm_init();
	void swarm_update(edge_frame_t edge_frame);
	void swarm_clear();
	SDL_Point* get_points() {
		return points;
	}

private:
	int bee_num;
	Bee* bees_ptr;
	SDL_Point* points;

};


#endif