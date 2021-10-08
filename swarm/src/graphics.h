#ifndef graphics_h
#define graphics_h

#include <opencv2/core.hpp>

#include "swarm.h"

bool graphics_initialize();
void graphics_dispose();

int graphics_render(const swarm_t *swarm, bool fps, bool debug, const cv::Mat3b *video_frame, const cv::Mat1w *depth_frame, const cv::Mat1b *edge_frame);

#endif /* graphics_h */
