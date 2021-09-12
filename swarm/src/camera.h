#ifndef camera_h
#define camera_h

#include <cstddef>
#include <cstdint>

#include "camera_frame.h"

bool camera_initialize();
void camera_dispose();

int camera_read_frame(video_frame_t *video_frame, depth_frame_t *depth_frame);

#endif /* camera_h */
