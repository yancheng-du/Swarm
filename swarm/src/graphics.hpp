#ifndef graphics_hpp
#define graphics_hpp

#include <opencv2/core.hpp>

#include "swarm.hpp"

bool graphics_initialize();
void graphics_dispose();

int graphics_render(const swarm_t &swarm, bool debug, const cv::Mat3b &video_frame, const cv::Mat1w &depth_frame, const cv::Mat1b &edge_frame, bool fps);

#endif /* graphics_hpp */
