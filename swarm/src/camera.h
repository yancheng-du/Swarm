#ifndef camera_h
#define camera_h

#include <opencv2/core.hpp>

bool camera_initialize();
void camera_dispose();

int camera_read_frame(cv::Mat3b *video_frame, cv::Mat1w *depth_frame, cv::Mat1b *edge_frame);

void get_vector_frame(cv::Mat1b* edge_frame, cv::Mat* x_vector_frame, cv::Mat* y_vector_frame, size_t field_size);
void init_field(size_t field_size);

#endif /* camera_h */
