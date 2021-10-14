#include <opencv2/core.hpp>
#include <opencv2/dnn.hpp>
#include <string>
#include <vector>
#include <iostream>
#include <filesystem>
#include <fstream>
#include "model.h"

// Initialize the parameters
const double confidence_threshold= 0.5; // Confidence threshold
const double nms_threshold= 0.4;  // Non-maximum suppression threshold
const int input_width= 416;        // Width of network's input image, Not actual image size just the network config, this is moderate speed/precision
const int input_height= 416;       // Height of network's input image, Not actual image size just the network config, this is moderate speed/precision

const std::string classes_file= "obj.names";
const std::string config_file= "yolov3_custom.cfg";
const std::string weights_file= "yolov3_custom_last (1).weights";






model_t::model_t()
{
	//Load names of classes to be detected by model
	classes= get_class_names();

	//Load the neural network
	network = cv::dnn::readNetFromDarknet(config_file, weights_file);
	network.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV); 
	// Right now setting to CPU, will have to research if we can leverage GPU if necessary
	network.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);

	std::cout << "Got through initialization";
	
}

std::vector<cv::Mat> model_t::get_gestures(cv::Mat frame) 
{
	
	//Create blob to pass to neural network
	cv::Mat4d blob; //**NEED TO CHECK IF THIS IS THE CORRECT DATATYPE FOR THIS**

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

	for (int i = 0; i < classes.size(); i++) {
		std::cout << classes.at(i) << ' ';
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

}

