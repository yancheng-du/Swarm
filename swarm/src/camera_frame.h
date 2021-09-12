#ifndef camera_frame_h
#define camera_frame_h

const size_t k_camera_width= 640;
const size_t k_camera_height= 480;

typedef uint8_t video_frame_t[k_camera_width*k_camera_height*3];
typedef uint16_t depth_frame_t[k_camera_width*k_camera_height];

#endif /* camera_frame_h */
