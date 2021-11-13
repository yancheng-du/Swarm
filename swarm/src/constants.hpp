#ifndef constants_hpp
#define constants_hpp

const int k_fps= 60;
const float k_dt= 1.0f/k_fps;

const int k_camera_width= 640;
const int k_camera_height= 480;

// subset of the camera frame used for edge detection
const int k_edge_width= k_camera_width;
const int k_edge_height= k_camera_width*9/16;
const int k_edge_x= 0;
const int k_edge_y= (k_camera_height-k_edge_height)/2;

const int k_depth_threshold= 1500; // in millimeters

const int k_simulation_width= 1920;
const int k_simulation_height= 1080;

const int k_bee_count= 8000;
const float k_bee_radius= 8.0f;

const int k_seconds_before_idle= 10;

#endif /* constants_hpp */
