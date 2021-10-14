#ifndef model_h
#define model_h

#include <string>
#include <vector>
#include <opencv2/core.hpp>
#include <opencv2/dnn.hpp>

class model_t
{
public:
	model_t();
	~model_t();

	std::vector<cv::Mat> get_gestures(cv::Mat frame);
	void do_gesture_loop();

	std::vector<std::string> classes;


private:
	std::vector<std::string> get_class_names();
	

	cv::dnn::Net network;

	std::vector<std::string> get_output_names(const cv::dnn::Net& net);
	int postprocess(cv::Mat& frame, const std::vector<cv::Mat>& outs);
};


#endif /* model_h */
