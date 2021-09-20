#ifndef graphics_h
#define graphics_h

#include "camera_frame.h"
#include "Swarm.h"

bool graphics_initialize();
void graphics_dispose();

int graphics_render(const video_frame_t *video_frame, const depth_frame_t *depth_frame);

int draw_bees(Swarm* bees);

#endif /* graphics_h */
