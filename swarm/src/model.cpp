#include <fstream>
#include <string>
#include <vector>

#include "model.hpp"

// Initialize the parameters
const float k_confidence_threshold= 0.1f;	// Confidence threshold
const float k_nms_threshold= 0.5f;			// Non-maximum suppression threshold
const int k_input_width= 416;				// Width of network's input image, Not actual image size just the network config, this is moderate speed/precision
const int k_input_height= 416;				// Height of network's input image, Not actual image size just the network config, this is moderate speed/precision

const int k_camera_width = 640;
const int k_camera_height = 480;

const int k_edge_width = k_camera_height * 9 / 16;

const std::string k_classes_filename= "res/gesture.names";
const std::string k_config_filename= "res/gesture.cfg";
const std::string k_weights_filename= "res/gesture.weights";

model_t::model_t()
{
	//Load names of classes to be detected by model
	classes= get_class_names();

	//Load the neural network
	network= cv::dnn::readNetFromDarknet(k_config_filename, k_weights_filename);
	network.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV); 
	// Right now setting to CPU, will have to research if we can leverage GPU if necessary
	network.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
}

void model_t::analyze_frame(const cv::Mat &frame, commands_t &commands)
{
	commands.clear();

	if (!frame.empty())
	{
		//std::vector<cv::Mat> outputs= get_gestures(frame);
		std::vector<cv::Mat> outputs = get_gestures(frame(cv::Rect((k_camera_width - k_edge_width) / 2, 0, k_edge_width, k_camera_height)));
		std::vector<command_t> gestures= postprocess(frame, outputs);

		// $TODO support more than one command at a time?

		for (int i= 0; i< gestures.size(); i++)
		{
			commands.push_back(gestures[i]);
		}
	}
}

// Remove the bounding boxes with low confidence using non-maxima suppression
std::vector<command_t> model_t::postprocess(const cv::Mat &frame, const std::vector<cv::Mat> &outs)
{
	std::vector<int> classIds;
	std::vector<float> confidences;
	std::vector<cv::Rect> boxes;

	for (size_t i= 0; i<outs.size(); ++i)
	{
		// Scan through all the bounding boxes output from the network and keep only the
		// ones with high confidence scores. Assign the box's class label as the class
		// with the highest score for the box.
		float* data= (float*)outs[i].data;
		for (int j= 0; j<outs[i].rows; ++j, data+= outs[i].cols)
		{
			cv::Mat scores= outs[i].row(j).colRange(5, outs[i].cols);
			cv::Point classIdPoint;
			double confidence;
			// Get the value and location of the maximum score
			cv::minMaxLoc(scores, 0, &confidence, 0, &classIdPoint);
			if (confidence>k_confidence_threshold)
			{
				int centerX= (int)(data[0] * frame.cols);
				int centerY= (int)(data[1]*frame.rows);
				int width= (int)(data[2]*frame.cols);
				int height= (int)(data[3]*frame.rows);
				int left= centerX - width/2;
				int top= centerY - height/2;

				classIds.push_back(classIdPoint.x);
				confidences.push_back((float)confidence);
				boxes.push_back(cv::Rect(left, top, width, height));
			}
		}
	}

	// Perform non maximum suppression to eliminate redundant overlapping boxes with
	// lower confidences
	std::vector<int> indices;
	cv::dnn::NMSBoxes(boxes, confidences, k_confidence_threshold, k_nms_threshold, indices);
	//if (indices.size()>0)
	//{
	//	return classIds[indices[0]];
	//}
	std::vector<command_t> gestures;
	for (int i= 0; i< indices.size(); i++) 
	{
		command_t command;
		int index= indices[i];
		int object_num = classIds[index];
		command.name= classes[object_num];
		command.confidence= confidences[index];
		cv::Rect box= boxes[index];
		command.bounding_box= cv::Rect(box.x, box.y, box.width, box.height);
		gestures.push_back(command);
	}
	return gestures;
}

std::vector<cv::Mat> model_t::get_gestures(const cv::Mat &frame)
{
	
	//Create blob to pass to neural network
	cv::Mat blob; //**NEED TO CHECK IF THIS IS THE CORRECT DATATYPE FOR THIS**

	//**THE TRUE IN THIS FUNCTION MAY NEED TO CHANGE TO FALSE DEPENDING ON IF INPUT FRAME IS RGB OR BGR**
	cv::dnn::blobFromImage(frame, blob, 1/255.0, cv::Size(k_input_width, k_input_height), cv::Scalar(0,0,0), true, false);
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

	std::ifstream in_file(k_classes_filename.c_str());
	std::string line;

	while (std::getline(in_file, line))
	{
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
		std::vector<int> output_layers= net.getUnconnectedOutLayers();

		//get the names of all the layers in the network
		std::vector<std::string> layer_names= net.getLayerNames();

		// Get the names of the output layers in names
		names.resize(output_layers.size());
		for (size_t i= 0; i<output_layers.size(); ++i)
			names[i]= layer_names[output_layers[i]-1];
	}
	return names;
}
