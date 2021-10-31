#ifndef camera_hpp
#define camera_hpp

#include <opencv2/core.hpp>

const int k_camera_width= 640;
const int k_camera_height= 480;

const int k_edge_width= k_camera_width;
const int k_edge_height= k_camera_width*9/16;
const int k_edge_x= 0;
const int k_edge_y= (k_camera_height-k_edge_height)/2;

bool camera_initialize();
void camera_dispose();

void camera_peek_video_frame(cv::Mat3b &video_frame);
int camera_consume_full_frame(cv::Mat3b &video_frame, cv::Mat1w &depth_frame, cv::Mat1b &edge_frame, bool idle);

int image_dist(const cv::Mat3b &video_frame, cv::Mat3b &last_video_frame);
void idle_check(const cv::Mat3b &video_frame, cv::Mat3b &last_video_frame, bool &idle);

int get_distance();
float get_avg_distance();

#endif /* camera_hpp */
