#ifndef camera_h
#define camera_h

#include <cstddef>
#include <cstdint>

const size_t k_camera_width= 640;
const size_t k_camera_height= 480;

typedef uint8_t RGB_frame_t[k_camera_width*k_camera_height*3];
typedef uint16_t depth_frame_t[k_camera_width*k_camera_height];

bool camera_initialize();
void camera_dispose();

int camera_read_frame(RGB_frame_t RBG_frame, depth_frame_t depth_frame);

#endif /* camera_h */
