#include <opencv2/core.hpp>
#include <opencv2/dnn.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/imgproc.hpp>
#include <string>
#include <vector>
#include <iostream>
#include<thread>

#include <filesystem>
#include <fstream>
#include "model.h"

// Initialize the parameters
const double confidence_threshold= 0.1; // Confidence threshold
const double nms_threshold= 0.5;  // Non-maximum suppression threshold
const int input_width= 416;        // Width of network's input image, Not actual image size just the network config, this is moderate speed/precision
const int input_height= 416;       // Height of network's input image, Not actual image size just the network config, this is moderate speed/precision

const std::string classes_file= "res/obj.names";
const std::string config_file= "res/yolov3_custom.cfg";
const std::string weights_file= "res/yolov3_custom_last (1).weights";

static std::thread *gesture_thread = NULL;




model_t::model_t()
{
	//Load names of classes to be detected by model
	classes= get_class_names();

	//Load the neural network
	network = cv::dnn::readNetFromDarknet(config_file, weights_file);
	network.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV); 
	// Right now setting to CPU, will have to research if we can leverage GPU if necessary
	network.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);

	gesture_thread = new std::thread([this] {this->do_gesture_loop(); });
	
}

void model_t::do_gesture_loop()
{
	cv::VideoCapture cap(0);
	cv::Mat frame;

	while (1) {

		cap >> frame;

		std::vector<cv::Mat> outputs = get_gestures(frame);
		int gesture_id = postprocess(frame, outputs);

		if (gesture_id != -1) 
		{
			std::cout << "Gesture: " << classes[gesture_id];
		}
		

	}
}

// Remove the bounding boxes with low confidence using non-maxima suppression
int model_t::postprocess(cv::Mat& frame, const std::vector<cv::Mat>& outs)
{
	std::vector<int> classIds;
	std::vector<float> confidences;
	std::vector<cv::Rect> boxes;

	for (size_t i = 0; i < outs.size(); ++i)
	{
		// Scan through all the bounding boxes output from the network and keep only the
		// ones with high confidence scores. Assign the box's class label as the class
		// with the highest score for the box.
		float* data = (float*)outs[i].data;
		for (int j = 0; j < outs[i].rows; ++j, data += outs[i].cols)
		{
			cv::Mat scores = outs[i].row(j).colRange(5, outs[i].cols);
			cv::Point classIdPoint;
			double confidence;
			// Get the value and location of the maximum score
			cv::minMaxLoc(scores, 0, &confidence, 0, &classIdPoint);
			if (confidence > confidence_threshold)
			{
				int centerX = (int)(data[0] * frame.cols);
				int centerY = (int)(data[1] * frame.rows);
				int width = (int)(data[2] * frame.cols);
				int height = (int)(data[3] * frame.rows);
				int left = centerX - width / 2;
				int top = centerY - height / 2;

				classIds.push_back(classIdPoint.x);
				confidences.push_back((float)confidence);
				boxes.push_back(cv::Rect(left, top, width, height));
			}
		}
	}

	// Perform non maximum suppression to eliminate redundant overlapping boxes with
	// lower confidences
	std::vector<int> indices;
	cv::dnn::NMSBoxes(boxes, confidences, confidence_threshold, nms_threshold, indices);
	if (indices.size() > 0)
	{
		return classIds[indices[0]];
	}
	else
	{
		return -1;
	}
}


std::vector<cv::Mat> model_t::get_gestures(cv::Mat frame) 
{
	
	//Create blob to pass to neural network
	cv::Mat blob; //**NEED TO CHECK IF THIS IS THE CORRECT DATATYPE FOR THIS**

	//**THE TRUE IN THIS FUNCTION MAY NEED TO CHANGE TO FALSE DEPENDING ON IF INPUT FRAME IS RGB OR BGR**
	cv::dnn::blobFromImage(frame, blob, 1/255.0, cv::Size(input_width, input_height), cv::Scalar(0,0,0), true, false);
	network.setInput(blob);

	std::vector<cv::Mat> outputs;
	network.forward(outputs, get_output_names(network));

	//Still need to do post-processing on the outputs of the neural network
	//Need to remove low confidence boxes
	//Need to perform NMS
	return outputs;
}

std::vector<std::string> model_t::get_class_names() 
{
	std::vector<std::string> classes;

	std::ifstream in_file(classes_file.c_str());
	std::string line;

	while (std::getline(in_file, line)) {
		classes.push_back(line);
	}

	return classes;

}

// Get the names of the output layers
std::vector<std::string> model_t::get_output_names(const cv::dnn::Net& net)
{
	static std::vector<std::string> names;
	if (names.empty())
	{
		//Get the indices of the output layers, i.e. the layers with unconnected outputs
		std::vector<int> output_layers = net.getUnconnectedOutLayers();

		//get the names of all the layers in the network
		std::vector<std::string> layer_names = net.getLayerNames();

		// Get the names of the output layers in names
		names.resize(output_layers.size());
		for (size_t i = 0; i < output_layers.size(); ++i)
			names[i] = layer_names[output_layers[i] - 1];
	}
	return names;
}

model_t::~model_t()
{
	gesture_thread->join();
	delete gesture_thread;
}

