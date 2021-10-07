#ifndef camera_h
#define camera_h

#include <opencv2/core.hpp>

bool camera_initialize();
void camera_dispose();

int camera_read_frame(cv::Mat3b *video_frame, cv::Mat1w *depth_frame, cv::Mat1b *edge_frame);

#endif /* camera_h */
