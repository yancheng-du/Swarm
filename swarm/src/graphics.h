#ifndef graphics_h
#define graphics_h

#include "swarm.h"

bool graphics_initialize();
void graphics_dispose();

int graphics_render(const video_frame_t *video_frame, const depth_frame_t *depth_frame, const edge_frame_t *edge_frame, const swarm_t *swarm);

#endif /* graphics_h */
