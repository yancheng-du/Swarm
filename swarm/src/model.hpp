#ifndef model_hpp
#define model_hpp

#include <string>
#include <vector>

#include <opencv2/core.hpp>
#include <opencv2/dnn.hpp>

#include "gesture.hpp"

class model_t
{
public:
	model_t();

	std::vector<cv::Mat> get_gestures(const cv::Mat &frame);
	void analyze_frame(const cv::Mat &frame, commands_t &commands);

	std::vector<std::string> classes;

private:
	std::vector<std::string> get_class_names();

	cv::dnn::Net network;

	std::vector<std::string> get_output_names(const cv::dnn::Net &net);
	int postprocess(const cv::Mat &frame, const std::vector<cv::Mat> &outs);
};


#endif /* model_hpp */
