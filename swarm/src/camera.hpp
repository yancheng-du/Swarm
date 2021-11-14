#ifndef camera_hpp
#define camera_hpp

#include <opencv2/core.hpp>

bool camera_initialize();
void camera_dispose();

void camera_peek_video_frame(cv::Mat3b &video_frame);
int camera_consume_full_frame(cv::Mat3b &video_frame, cv::Mat1w &depth_frame, cv::Mat1b &edge_frame);

#endif /* camera_hpp */
