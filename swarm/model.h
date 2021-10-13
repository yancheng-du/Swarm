#ifndef model_h
#define model_h

#include <string>
#include <vector>
#include <opencv2/core.hpp>

class model_t
{
public:
	model_t();
	~model_t();

	std::vector<cv::Mat> get_gestures(cv::Mat frame);


private:
	std::vector<std::string> get_class_names();
	std::vector<std::string> classes;

	cv::dnn::Net network;

	std::vector<std::string> get_output_names(const cv::dnn::Net& net);

};


#endif /* model_h */
