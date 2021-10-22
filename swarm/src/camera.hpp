#ifndef camera_hpp
#define camera_hpp

#include <opencv2/core.hpp>

const int k_camera_width = 640;
const int k_camera_height = 480;

const int k_cropped_camera_width = 270;
const int k_cropped_camera_height = 480;

const int k_edge_width = k_camera_height * 9 / 16;

bool camera_initialize();
void camera_dispose();

void camera_peek_video_frame(cv::Mat3b &video_frame);
int camera_consume_full_frame(cv::Mat3b &video_frame, cv::Mat1w &depth_frame, cv::Mat1b &edge_frame, bool idle);

void get_vector_frame(const cv::Mat1b &edge_frame, cv::Mat &x_vector_frame, cv::Mat &y_vector_frame, int field_size);
void init_field(int field_size);

int image_dist(const cv::Mat3b &video_frame, cv::Mat3b &last_video_frame);
void idle_check(const cv::Mat3b &video_frame, cv::Mat3b &last_video_frame, bool &idle);

int get_distance();
float get_avg_distance();

#endif /* camera_hpp */
