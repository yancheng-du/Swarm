#include "libfreenect.hpp"
#include <iostream>
#include <utility>
#include <vector>
#include <cmath>
#include <pthread.h>
#include <opencv2/opencv.hpp>
#include <fstream>
#include <chrono>
#include <stdlib.h>
#include "BeeHandle.cpp"
#include "MyFreenectDevice.hpp"
#include "AudioHandler.hpp"
#include "./graphics/graphics_module.hpp"
#include "utils.h"

#include "tensorflow/cc/ops/const_op.h"
#include "tensorflow/cc/ops/image_ops.h"
#include "tensorflow/cc/ops/standard_ops.h"
#include "tensorflow/core/framework/graph.pb.h"
#include "tensorflow/core/graph/default_device.h"
#include "tensorflow/core/graph/graph_def_builder.h"
#include "tensorflow/core/lib/core/threadpool.h"
#include "tensorflow/core/lib/io/path.h"
#include "tensorflow/core/lib/strings/stringprintf.h"
#include "tensorflow/core/platform/init_main.h"
#include "tensorflow/core/public/session.h"
#include "tensorflow/core/util/command_line_flags.h"

using namespace cv;
using namespace std;

using tensorflow::Flag;
using tensorflow::Tensor;
using tensorflow::Status;
using tensorflow::string;
using tensorflow::int32;

//convert all values in mat > threshold to 0
void filter(cv::Mat mat, int threshold){
  for(signed i = 0; i<mat.rows; i++){
		for(signed j = 0; j<mat.cols; j++){
			int mat_val = mat.data[mat.step[0]*i + mat.step[1]* j];
			if(mat_val == 255 or mat_val > threshold){
				mat.data[mat.step[0]*i + mat.step[1]* j] = 0;
			}
		}
	}
}

//
//used to test whether a string is an integer -> for arg parsing
//https://stackoverflow.com/questions/2844817/how-do-i-check-if-a-c-string-is-an-int
inline bool isInteger(const std::string & s)
{
   if(s.empty() || ((!isdigit(s[0])) && (s[0] != '-') && (s[0] != '+'))) return false;

   char * p;
   strtol(s.c_str(), &p, 10);

   return (*p == 0);
}

/*
given a list of contours, create a list of points by concatenating each contour
Also, drop every <prop> contours
*/
vector<Point> drop_contours_1d(vector<vector<Point>> contours, int prop){
	vector<Point> ret_arr;

	for(unsigned i = 0; i < contours.size(); i++){
		for(unsigned j = 0; j < contours.at(i).size(); j++){
			if(j%prop == 0){
				ret_arr.push_back(contours.at(i).at(j));
			}
		}
	}

	return ret_arr;
}

/*
potential arguments :
--bees <number of bees>
--time		(if present, times the exection)
--videos	(if present, shows the videos for rgb in, background subtracted and edges)
--scale <scale> (currently multiplied by 320x240 to get screen size)
--size <size>   (the size of each bee)
--soundb <sound divisor> (the sound divisor used for the audiohandler)
--steps (shows intermediate images)
--fullscreen (displays the window to entire screen)
--canny <int> <int> (sets the upper and lower thresholds for edge detection)
--stepsize <stepsize> (the distance bees can travel in one frame)
*/
int main(int argc, char **argv) {

	//various parameters
	int width = 300;
	int height = 480;
	int down_width = 300; //we resize our input arrays for faster computation
	int down_height = 480;
	int contour_drop = 1; //we keep 1/<contour_drop> contours
	int depth_threshold = 1500; //threshold depth in mm
  float scale = 3.0f; //scale for graphics window
  float bee_size = 2.0f; //size of each bee
	int num_bees = 1200; //number of bees
  int bee_total = 0; //time spent on bee module
	bool time_it = false; //whether we use timing or not
  int sound_divisor = 20; //parameter for audiohandler
  int randgen = 1;
  bool steps = false; //whether or not to show intermediate images
  bool fullscreen = false; //whether the graphics window should occupy the entire screen
  int c_lower_thres = 50; //lower threshold for canny edge detection
  int c_upper_thres = 100;  //upper thrshold for canny edge detection
  int step_size = 3;
  bool gesture = false;
  int frame_counter = 50;
  int count_frames = 0;

  	//set variables for timing
	chrono::time_point<std::chrono::high_resolution_clock> time_start;
	chrono::time_point<std::chrono::high_resolution_clock> time_stop;
	chrono::time_point<std::chrono::high_resolution_clock> bee_start;
	chrono::time_point<std::chrono::high_resolution_clock> bee_stop;
	int iterations = 0;

	//variables for frame limiting
	chrono::system_clock::time_point limiter_start = chrono::system_clock::now();
	chrono::system_clock::time_point limiter_end = chrono::system_clock::now();

	//variables for video recording
	bool display_qr = false;
	chrono::system_clock::time_point qr_start;
	chrono::system_clock::time_point qr_current;

  	int contour_count = 0;
  	vector<Point> set_contour;

	//argument parsing
	if(argc > 1){
		for(int i = 0; i < argc; i++){
			if(String(argv[i]).compare("--bees")==0 && argc>(i+1) && isInteger(argv[i+1])){
				num_bees = stoi(String(argv[i+1]));
			}

      if(String(argv[i]).compare("--time")==0){
        time_it = true;
        bee_total = 0;
      }

      if(String(argv[i]).compare("--scale")==0 && argc>(i+1)){
        scale = stof(String(argv[i+1]));
      }
      if(String(argv[i]).compare("--gesture")==0){
        gesture = true;
      }

      if(String(argv[i]).compare("--size")==0 && argc>(i+1)){
        bee_size = stof(String(argv[i+1]));
      }

      if(String(argv[i]).compare("--stepsize")==0 && argc>(i+1)){
        step_size = stof(String(argv[i+1]));
      }

      if(String(argv[i]).compare("--soundb")==0 && argc>(i+1) && isInteger(argv[i+1])){
        sound_divisor = stoi(String(argv[i+1]));
      }

      if(String(argv[i]).compare("--canny")==0 && argc>(i+2) && isInteger(argv[i+1]) && isInteger(argv[i+2])){
        c_lower_thres = stoi(String(argv[i+1]));
        c_upper_thres = stoi(String(argv[i+2]));
      }

      if(String(argv[i]).compare("--steps")==0){
        steps = true;
      }

      if(String(argv[i]).compare("--fullscreen")==0){
        fullscreen = true;
      }

    }
  }

	//create bee handler for calculating bee dynamics
  //xwidth - width of matrix
  //ywidth - height of matrix
  //stepSize - how far a bee will move in a single frame
  //randomFactor - how random the bee movement is [-pi/2, pi/2]
  //numThreads - how many threads
  //storedFrames - how many frames are stored for averaging
  //avgPercent - % of the stored frames that must contain a contour before considering that contour
	BeeHandle bee_handle = BeeHandle(down_width, down_height, step_size, 0.2, 4, 15, (double) 1/5);
	//bee_handle.add_bees(num_bees);
  for (int i = 0; i < num_bees; i++){
    bee_handle.addP();
  }

	//BeeHandle bee_handle = BeeHandle(down_width, down_height);
	//bee_handle.add_bees(num_bees);

	AudioHandler audio = AudioHandler(down_width, down_height);
  //audio.play_sound(32);

	//seed our random number generator
	RNG rng(1235);

	//create the matrices we'll use
	Mat depthIn = Mat::zeros(Size(640,480), CV_16UC1); //kinect always reads in at this size
	Mat rgbIn = Mat::zeros(Size(640,480), CV_8UC3);
  Mat cropDepthIn = Mat::zeros(Size(width, height), CV_16UC1);
  Mat cropRgbIn = Mat::zeros(Size(width, height), CV_8UC3);
	Mat depth_down = Mat::zeros(Size(down_width,down_height),CV_16UC1);
	Mat rgb_down = Mat::zeros(Size(down_width,down_height),CV_8UC3);
	Mat depthf = Mat::zeros(Size(down_width,down_height),CV_8UC3);
	Mat mask = Mat::zeros(Size(down_width,down_height), CV_8UC3);
	Mat grayMat = Mat::zeros(Size(down_width,down_height), CV_8UC1);
	Mat cannyResult;
	Mat lastFrame = Mat::zeros(Size(down_width, down_height), CV_8UC3);
	Mat outMat = Mat::zeros(Size(width, height),CV_8UC1);
	Mat finalFrame = Mat::zeros(Size(down_width, down_height), CV_8UC1);

	bool expected = false;
	bool was_expected = false;
	

  //path variables
  string rootdir = "./pbfiles/";
  string hand_labels = "labels_map.pbtxt";
	string hand_graph = "frozen_inference_graph.pb";
	string gesture_graph = "output_graph.pb";
	string inputLayer = "image_tensor:0";
	vector<string> outputLayer = {"detection_boxes:0", "detection_scores:0", "detection_classes:0", "num_detections:0"};

	//load and initialize the hand model
	std::unique_ptr<tensorflow::Session> session;
	string graph_path = tensorflow::io::JoinPath(rootdir, hand_graph);
	LOG(INFO) << "handgraphPath:" << graph_path;
	Status load_graph_status = loadGraph(graph_path, &session);
	if (!load_graph_status.ok()) {
	    LOG(ERROR) << "loadGraph(): ERROR" << load_graph_status;
	    return -1;
	}

	//load hand labels
	std::map<int, std::string> labels_map = std::map<int,std::string>();
	Status read_labels_status = readLabelsMapFile(tensorflow::io::JoinPath(rootdir, hand_labels), labels_map);
	if (!read_labels_status.ok()) {
	    LOG(ERROR) << "readLabelsMapFile(): ERROR" << read_labels_status;
	    return -1;
	}

	//load and initialize the gesture model
	std::unique_ptr<tensorflow::Session> session2;
	string graph_path2 = tensorflow::io::JoinPath(rootdir, gesture_graph);
	LOG(INFO) << "handgraphPath:" << graph_path2;
	Status load_graph_status2 = loadGraph(graph_path2, &session2);
	if (!load_graph_status2.ok()) {
	    LOG(ERROR) << "loadGraph(): ERROR" << load_graph_status2;
	    return -1;
	}

	Mat resized;
	bool decrement = false;

	//tensor shape
	Tensor tensor;
    	vector<Tensor> outputs;
	double thresholdScore = 0.5;
    	double thresholdIOU = 0.8;
	tensorflow::TensorShape shape = tensorflow::TensorShape();
    	shape.AddDim(1);
    	shape.AddDim(down_height);
    	shape.AddDim(down_width);
    	shape.AddDim(3);


	//create all the vectors that we'll need
	vector<vector<Point>> contours;
	vector<Point> flat_contours;
	vector<Vec4i> hierarchy;
	vector<Point> bee_positions;
	vector<Point> bee_attractors;
	vector<int> landed;
	//vector<bool> bee_trigger;

	vector<vector<Point>> capturedContours;
	vector<Point> captured_flat_contours;
	vector<Vec4i> capturedHierarchy;

	//create our connection to the connect
	Freenect::Freenect freenect;
	MyFreenectDevice& device = freenect.createDevice<MyFreenectDevice>(0);

	//start up our kinect
	device.startVideo();
	device.startDepth();

	/*sometimes frames are dropped, this variable is used to tell if a
	frame has been dropped, or if there's just nothing being
	recorded*/
	int num_dropped = 0;

	//initialization for graphics module
	GraphicsModule gm (num_bees, down_width, down_height,
		                 scale, bee_size/10.0f, fullscreen,
		                 "./graphics/abee.png",
	                   "./graphics/");
	vector<int> bee_x (num_bees);
	vector<int> bee_y (num_bees);
	vector<int> bee_stage (num_bees);

  // 0 - 7 starting north going clockwise
	vector<int> bee_dir (num_bees);
  vector<int> bee_landed;

  //windows for displaying intermediates
  if(steps){
    cv::namedWindow("rgb", cv::WINDOW_AUTOSIZE);
    cv::namedWindow("masked", cv::WINDOW_AUTOSIZE);
    cv::namedWindow("edges", cv::WINDOW_AUTOSIZE);
  }
  //cv::waitKey(0);

	if(time_it)
		time_start = chrono::high_resolution_clock::now();

	//main loop
  do {

		//get time for frame limiting
		limiter_start = chrono::system_clock::now();
		chrono::duration<double, std::milli> work_time = limiter_start - limiter_end;
		if(work_time.count() < 33.0){ //30 FPS
			chrono::duration<double, std::milli> delta_ms(33.0 - work_time.count());
            		auto delta_ms_duration = chrono::duration_cast<chrono::milliseconds>(delta_ms);
            		this_thread::sleep_for(chrono::milliseconds(delta_ms_duration.count()));
		}

		//time and wait if we are doing frame limiting
		limiter_end = chrono::system_clock::now();
		std::chrono::duration<double, std::milli> sleep_time = limiter_end - limiter_start;

		//reset our matrices
		outMat = Scalar(0);
		finalFrame = Scalar(0);

		//get one frame of video and one frame of depth from the kinect
		device.getVideo(rgbIn);
		device.getDepth(depthIn);

    //crop frame to fit on vertical screen
    int h_left = (640 - width)/2;
    int h_right = 640 - (640 - width)/2;
    int v_left = (480 - height)/2;
    int v_right = 480 - (480 - height)/2;
    cropRgbIn = rgbIn(cv::Rect(cv::Point(h_left, v_left), cv::Point(h_right, v_right))).clone();
    cropDepthIn = depthIn(cv::Rect(cv::Point(h_left, v_left), cv::Point(h_right, v_right))).clone();

		//gesture detection
		Rect rec;

		//LOG(INFO)<<"show captured"<<endl;

		if(iterations%15==0 && gesture==true) {
			cvtColor(rgb_down, rgb_down, COLOR_BGR2RGB);

			// Convert mat to tensor
			tensor = Tensor(tensorflow::DT_FLOAT, shape);
			Status read_tensor_status = readTensorFromMat(rgb_down, tensor);
			if (!read_tensor_status.ok()) {
		 	   LOG(ERROR) << "Mat->Tensor conversion failed: " << read_tensor_status;
	   	         return -1;
			}

	 	      	// Run the graph on tensor
	 	       	outputs.clear();
	 	       	Status runStatus = session->Run({{inputLayer, tensor}}, outputLayer, {}, &outputs);
	 	       	if (!runStatus.ok()) {
	 	           LOG(ERROR) << "Running model failed: " << runStatus;
	 	           return -1;
	   	     	}

	    	    	// Extract results from the outputs vector
			tensorflow::TTypes<float>::Flat scores = outputs[1].flat<float>();
			//tensorflow::TTypes<float>::Flat classes = outputs[2].flat<float>();
			//tensorflow::TTypes<float>::Flat numDetections = outputs[3].flat<float>();
			tensorflow::TTypes<float, 3>::Tensor boxes = outputs[0].flat_outer_dims<float,3>();

			vector<size_t> goodIdxs = filterBoxes(scores, boxes, thresholdIOU, thresholdScore);

			// Draw boxes and captions
			cvtColor(rgb_down, rgb_down, COLOR_BGR2RGB);
	 		// LOG(INFO)<<"rgb_down cols:"<<rgb_down.cols<<endl;
			// LOG(INFO)<<"rgb_down height:"<<rgb_down.size().height<<endl;

			detect(rec, session2, rgb_down, scores, boxes, goodIdxs, &expected);
			//if(iterations%30==0){expected = true;}else{expected = false;}
			if(expected){
				audio.play_sound(iterations%32+1);
				was_expected = true;
				count_frames = 30;
			}
		}

		
		//resize input image and depth for decreased computation
		cv::resize(cropRgbIn, rgb_down, Size(down_width, down_height));
		cv::resize(cropDepthIn, depth_down, Size(down_width, down_height));

		//normalize depth to 0-255 range and filter every depth past our threshold
		depth_down.convertTo(depthf, CV_8UC3, 255.0/10000.0); //kinect caps out at 10000 mm
		filter(depthf, depth_threshold*255.0/10000.0); //remove background

		//background subtraction, filter our video mased on our depth thresholding
		cv::threshold(depthf, mask, 1, 255, THRESH_BINARY);
    rgb_down.setTo(Scalar(64, 177, 0), 255-mask);
    cv::cvtColor(rgb_down, outMat, cv::COLOR_BGR2GRAY);

		//find edges and contours
		cv::medianBlur(outMat, outMat, 3);
		if(was_expected){
			Mat roi = outMat(rec);
			cv::Canny(roi, cannyResult, c_lower_thres, c_upper_thres, 3);
		}else{
    			cv::Canny(outMat, cannyResult, c_lower_thres, c_upper_thres, 3);
		}
    		cv::findContours(cannyResult, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_TC89_L1, cv::Point(0,0));
		flat_contours = drop_contours_1d(contours, contour_drop);
		int old_size = flat_contours.size();

		// Duplicate edges randomly for better outlines
		for(int i=0; i < old_size; i++) {
			int duplicate_max = 5;
			int num_to_duplicate = (rand() % duplicate_max);
			for(int j=0; j<num_to_duplicate; j++) {
				int x_diff = (rand()%3)-1;
				int y_diff = (rand()%3)-1;
				int new_x = flat_contours.at(i).x + x_diff;
				int new_y = flat_contours.at(i).y + y_diff;
				if(new_x >= 0 && new_y >= 0)
					if(new_x < down_width && new_y < down_height)
						flat_contours.push_back(cv::Point(new_x, new_y));

			}
		}
		//cout << "bees: " << num_bees << std::endl;
		//cout << "contours: " << flat_contours.size() << std::endl << std::endl;

		//if we have a dropped frame, check if it's a one off, or if there's nothing
		//being recorded
		if(flat_contours.size() < 100){
			if(num_dropped<5){
				num_dropped++;
				continue;
			}
		}else{
			num_dropped = 0;
		}

		if(steps){
			cv::imshow("rgb", cropRgbIn);
			cv::waitKey(1);
			cv::imshow("masked", outMat);
			cv::waitKey(1);
			cv::imshow("edges", cannyResult);
			cv::waitKey(1);
		}

		//start timer for bee module if timing is enabled
		if(time_it){
			bee_start = chrono::high_resolution_clock::now();
		}

    //if(1000 > contour_count){
      //printf("%d\n", contour_count);
    //  contour_count++;
    //  bee_handle.addAttractorsAvg(flat_contours);
    //} else if (contour_count == 1000){
    //  printf("%d\n", contour_count);
    //  contour_count++;
    //  set_contour = flat_contours;
    //  bee_handle.addAttractorsAvg(set_contour);
    //} else if (contour_count > 2000){
    //  bee_handle.addAttractorsAvg(flat_contours);
    //} else {
    //  //printf("%d\n", contour_count);
    //  contour_count++;
    //  bee_handle.addAttractorsAvg(set_contour);
    //}

		//flatten contours and add as "flowers" to bee_handle
		//bee_handle.add_flowers(flat_contours);

		if(was_expected){
			count_frames--;
			
		}
		if(count_frames==0 && was_expected ==true){
			was_expected = false;
			decrement = true;
		}
		if(decrement==false){
			bee_handle.addAttractorsAvg(flat_contours);
		}else{
			frame_counter--;
		}
		if(frame_counter==0){
			decrement = false;
			frame_counter = 50;
		}
		bee_handle.updatePoints();


		int new_size = flat_contours.size();
		for(int i=0; i<(new_size-old_size); i++) {
			flat_contours.pop_back();
		}

		//get bee positions
		bee_positions.clear();

		//bee_positions = bee_handle.get_bees();
    bee_positions = bee_handle.getPoints();
    //bee_positions = flat_contours;
    //std::vector<Point> combined(flat_contours);
    //std::vector<Point> bh_points = bee_handle.getPoints();
    //combined.insert(combined.end(), bh_points.begin(), bh_points.end());
    //bee_positions = combined;
    bee_dir = bee_handle.get_dirs();
    bee_landed = bee_handle.get_landed();
    for(int i = 0; i < num_bees/sound_divisor; i++){
      if(bee_landed[i] == 1){

	//sounds same as below
        if(bee_positions[i].x < down_width/2 && bee_positions[i].y < down_height/2){
          randgen = rand() % 8;
          audio.set_point(randgen,bee_positions[i].x,bee_positions[i].y);
          //audio.play_sound(randgen);
        }

	//sounds like two sounds - bwooiii and dn
        if(bee_positions[i].x < down_width/2 && bee_positions[i].y >= down_height/2){
          randgen = rand() % 8 + 8;
          audio.set_point(randgen,bee_positions[i].x,bee_positions[i].y);
        //  audio.play_sound(randgen);
        }

	//buzzing
        if(bee_positions[i].x >= down_width/2 && bee_positions[i].y < down_height/2){
          randgen = rand() % 8 + 16;
          audio.set_point(randgen,bee_positions[i].x,bee_positions[i].y);
        //  audio.play_sound(randgen);
        }

	//chimes
        if(bee_positions[i].x >= down_width/2 && bee_positions[i].y >= down_height/2){
          randgen = rand()%(31-25 + 1) + 25;
          audio.set_point(randgen,bee_positions[i].x,bee_positions[i].y);
          audio.play_sound(randgen);
        }
      }

    }

		//update our graphics module
		for(int i = 0; i < num_bees; i++){
			bee_x[i] = bee_positions[i].x;
			bee_y[i] = bee_positions[i].y;
			bee_stage[i] = (bee_stage[i]+1)%12;
			//bee_dir[i] = (bee_dir[i]+1)%8;
		}
		gm.update_particles(bee_x, bee_y, bee_stage, bee_dir);
		display_qr = gm.update_display(true) || display_qr;

		//updates for qr code
		if(display_qr){
			qr_current = chrono::system_clock::now();
			chrono::duration<double, std::milli> qr_time = qr_current - qr_start;
	    		gm.update_qr(true, "./graphics/video.png", (int)down_width/10, (int)down_height/10, 5.0f);
        	if(qr_time.count() >= 20000.0){ //delete qr code
            gm.update_qr(false, "", 320, 240, 2.0f);
         		display_qr = false;
  				  system("sudo rm ./graphics/video.png");
  			  }
		}else{
			qr_start = chrono::system_clock::now();
		}

		//end timer for bees if timing is enabled
		if(time_it){
			bee_stop = chrono::high_resolution_clock::now();
			auto temp_bee_total = chrono::duration_cast<chrono::microseconds>(bee_stop - bee_start);
			bee_total += (int)temp_bee_total.count();
		}

		iterations++;
		//LOG(INFO)<<"frames: "<<iterations<<endl;
	}while(!gm.should_close());



	//output the results of our timing
	if(time_it){
		time_stop = chrono::high_resolution_clock::now();
		auto total = chrono::duration_cast<chrono::microseconds>(time_stop-time_start);
		cout<<"frames: "<<iterations<<endl;
		cout<<"Frames per second: "<<(float)iterations/(total.count()/1000000.0)<<endl;
		cout<<"Percentage bee module: "<<100*(float)bee_total/total.count()<<"%\n";
		cout<<"Percentage edge module: "<<100*(float)(total.count()-bee_total)/total.count()<<"%\n";
	}

	//clean up everything we have
	audio.delete_sources();
	device.stopVideo();
	device.stopDepth();
	finalFrame.release();
	outMat.release();
	depthIn.release();
	rgbIn.release();
	depth_down.release();
	rgb_down.release();
	depthf.release();
	mask.release();
	grayMat.release();
	cannyResult.release();
	lastFrame.release();

	//release debugging windows
	if(steps){
		cv::destroyWindow("rgb");
		cv::destroyWindow("masked");
		cv::destroyWindow("edges");
	}
	return 0;
}
