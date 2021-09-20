
#include "Swarm.h"



Bee::Bee() {}
Bee::~Bee() {}

void Bee::bee_init() {
	x = rand() % k_camera_width;
	y = rand() % k_camera_height;
	v_x = (rand() % BEE_VELOCITY - BEE_VELOCITY / 2);
	v_y = (rand() % BEE_VELOCITY - BEE_VELOCITY / 2);
	pos.x = x;
	pos.y = y;
}

void Bee::bee_update(edge_frame_t edge_frame) {
	if (edge_frame[y* k_camera_width + x] != 0) {
		v_x = 0;
		v_y = 0;
	}
	else {
		//bees have an 2% chance to
		if (rand() % 100 < 2) {
			v_x += (rand() % BEE_VELOCITY - BEE_VELOCITY / 2);
			v_y += (rand() % BEE_VELOCITY - BEE_VELOCITY / 2);
		}

	}

	if (x + v_x < 0 || x + v_x >= k_camera_width) {
		v_x = -v_x;
	}

	if (y + v_y < 0 || y + v_y >= k_camera_height) {
		v_y = -v_y;
	}

	x += v_x;
	y += v_y;

	pos.x = x;
	pos.y = y;
}


Swarm::Swarm() {}
Swarm::~Swarm() {}

void Swarm::swarm_init() {
	bee_num = BEE_NUM;
	bees_ptr = new Bee[bee_num];
	points = new SDL_Point[bee_num];
	for (int i = 0; i < bee_num; i++) {
		bees_ptr[i].bee_init();
		points[i] = bees_ptr[i].get_pos();
	}
}

void Swarm::swarm_update(edge_frame_t edge_frame) {
	for (int i = 0; i < bee_num; i++) {
		bees_ptr[i].bee_update(edge_frame);
		points[i] = bees_ptr[i].get_pos();
	}
}

void Swarm::swarm_clear() {
	delete(bees_ptr);
	delete(points);

}

