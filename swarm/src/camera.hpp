#ifndef camera_hpp
#define camera_hpp

#include <opencv2/core.hpp>

bool camera_initialize();
void camera_dispose();

void camera_peek_video_frame(cv::Mat3b *video_frame);
int camera_consume_full_frame(cv::Mat3b *video_frame, cv::Mat1w *depth_frame, cv::Mat1b *edge_frame, bool idle);

void get_vector_frame(cv::Mat1b *edge_frame, cv::Mat *x_vector_frame, cv::Mat *y_vector_frame, int field_size);
void init_field(int field_size);

int image_dist(cv::Mat3b *video_frame, cv::Mat3b *last_video_frame);
void idle_check(cv::Mat3b *video_frame, cv::Mat3b *last_video_frame, bool *idle);

#endif /* camera_hpp */
