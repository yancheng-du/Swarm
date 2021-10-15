#ifndef camera_h
#define camera_h

#include <opencv2/core.hpp>

bool camera_initialize();
void camera_dispose();

int camera_read_frame(cv::Mat3b *video_frame, cv::Mat1w *depth_frame, cv::Mat1b *edge_frame, bool idle);
int idle_camera_read_frame(cv::Mat3b *video_frame, cv::Mat1w *depth_frame, cv::Mat1b *idle_edge_frame);

void get_vector_frame(cv::Mat1b* edge_frame, cv::Mat* x_vector_frame, cv::Mat* y_vector_frame, size_t field_size);
void init_field(size_t field_size);

int image_dist(cv::Mat3b* video_frame, cv::Mat3b* last_video_frame);
void idle_check(cv::Mat3b* video_frame, cv::Mat3b* last_video_frame,bool* idle);
#endif /* camera_h */
