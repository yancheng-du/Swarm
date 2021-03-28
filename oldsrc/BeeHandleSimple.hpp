#include <iostream>
#include <vector>
#include <opencv.hpp>
#include <thread>
#include <math.h>

#define NUM_THREADS 8

// number of directions to average
#define DIR_MEMORY 10

using namespace std;

class BeeHandle
{
private:
  int move_dist;
  int max_x;
  int max_y;
  vector<cv::Point> bees;
  vector<cv::Point> flowers;
  thread threads [NUM_THREADS];
  vector<vector<int>> past_dirs;
  vector<int> dirs;
  vector<int> landed;
  int sound_divisor;

  void join_threads() {
  	for(int i=0; i<NUM_THREADS; i++) {
  		threads[i].join();
  	}
  }

  void set_threads() {
  	for(int i=0; i<NUM_THREADS; i++) {
  		threads[i] = thread(&BeeHandle::move_bee, this, i);
  	}
  }

  // Map an increment ∈ {-1, 0, 1} in x and y direction to direction number ∈ [0,7]
  // Assumes y increases as you move down the screen
  int x_y_to_dir(int x, int y) {
    if(x==-1) {
      if(y==-1) {
        return 7;
      }
      else if(y==0) {
        return 6;
      }
      else {
        return 5;
      }
    }
    else if(x==0) {
      if(y==-1) {
        return 0;
      }
      // randomize direction if the bee didn't move
      else if(y==0) {
        return (rand() % 3)-1;
      }
      else {
        return 4;
      }
    }
    else {
      if(y==-1) {
        return 1;
      }
      else if(y==0) {
        return 2;
      }
      else {
        return 3;
      }
    }
  }

  void move_bee(int thread_num) {
  	int bee_per_thread = bees.size()/NUM_THREADS;
  	int start = bee_per_thread*thread_num;
  	int end = start + bee_per_thread;
  	for(int i=start; i<end; i++) {
  	  bool did_land1 = false;
  	  bool did_land2 = false;
      int move_x = (rand() % 3)-1;
      int move_y = (rand() % 3)-1;
      int newX = (this->bees.at(i).x + move_x*move_dist)%max_x;
      int newY = (this->bees.at(i).y + move_y*move_dist)%max_y;

      if(newX < 0)
        newX = max_x-1;
      if(newY < 0)
        newY = max_y-1;

      cv::Point new_pos = cv::Point(newX, newY);

      float currPotential = get_potential(this->bees.at(i),&did_land1);
      float newPotential = get_potential(new_pos,&did_land2);
      if(newPotential > currPotential){
        bees.at(i) = new_pos;

        int dir = x_y_to_dir(move_x, move_y);
        // erase oldest direction
        this->past_dirs.at(i).pop_back();
        // insert new direction
        auto it = this->past_dirs.at(i).begin();
        this->past_dirs.at(i).insert(it, dir);

    		if(did_land2 && i%sound_divisor == 0){
    			landed.at(i/sound_divisor) = landed.at(i/sound_divisor)+1;
    		}

    	  else if(!did_land2 && i%sound_divisor == 0){
    			landed.at(i/sound_divisor) = 0;
    		}
      }
  	  else {
    		if(did_land1 && i%sound_divisor == 0) {
    			landed.at(i/sound_divisor) = landed.at(i/sound_divisor)+1;
    		}
  	  	else if(!did_land1 && i%sound_divisor == 0) {
			     landed.at(i/sound_divisor) = 0;
  		  }
      }

  	}
  }
public:
  BeeHandle(){
    max_x = 320;
    max_y = 240;

  }

  BeeHandle(int maxX, int maxY, int sound_div){
    max_x = maxX;
    max_y = maxY;
    sound_divisor = sound_div;
  }

  void add_bees(vector<cv::Point> locations){
    for(unsigned i = 0; i < locations.size(); i++){
      this->bees.push_back(locations.at(i));
      // randomize current direction
      int dir = (int)(rand()%8);
      this->dirs.push_back(dir);
      // initialize direction memory with random directions
      vector<int> bee_past_dirs;
      for(int j=0; j<DIR_MEMORY; j++) {
        int dir = (int)(rand()%8);
        bee_past_dirs.push_back(dir);
      }
      this->past_dirs.push_back(bee_past_dirs);
    }
  }

  void add_bees(int num_bees){
	landed.resize(num_bees/sound_divisor);
	std::fill(landed.begin(), landed.end(), 0);

    for(int i = 0; i < num_bees; i++){
      cv::Point p1 = cv::Point((int)(rand()%max_x), (int)(rand()%max_y));
      this->bees.push_back(p1);
      // randomize current direction
      int dir = (int)(rand()%8);
      this->dirs.push_back(dir);
      // initialize direction memory with random directions
      vector<int> bee_past_dirs;
      for(int j=0; j<DIR_MEMORY; j++) {
        int dir = (int)(rand()%8);
        bee_past_dirs.push_back(dir);
      }
      this->past_dirs.push_back(bee_past_dirs);
    }
  }

  void add_flowers(vector<cv::Point> locations){
    flowers = locations;
  }

  vector<cv::Point> get_bees(){
    return this->bees;
  }

  vector<int> get_landed(){
    return this->landed;
  }

  // Compute distance as the average of the last DIR_MEMORY directions
  vector<int> get_dirs() {
    for(int i=0; i<(int)past_dirs.size(); i++) {
      int sum = 0.0;
      for(int j=0; j<(int)past_dirs.at(0).size(); j++) {
        sum += past_dirs.at(i).at(j);
      }
      dirs.at(i) = round(sum/DIR_MEMORY);
    }
    return dirs;
  }

  void clear_flowers(){
    this->flowers.clear();
  }

  void clear_bees(){
    this->bees.clear();
    this->dirs.clear();
    this->past_dirs.clear();
  }

  int distance(cv::Point p1, cv::Point p2){
    return  (int)cv::norm(p1-p2);
  }

  float get_potential(cv::Point p, bool *did_land){
    float cur_potential = 0;
    int resistance_str = 2000;
    int attraction_str = 10000;
    int bee_stride = 10;
    int flower_stride = 1;
    int random_off_bee = rand()%bee_stride;
    int random_off_flower = rand()%flower_stride;

    for(unsigned i = random_off_bee; i < bees.size(); i+=bee_stride){
      int dist = distance(bees.at(i), p);
      if(dist != 0)
        cur_potential -=  (float)resistance_str*bee_stride/(dist);
      else
        cur_potential -= resistance_str*bee_stride;

    }

    for(unsigned i = random_off_flower; i < flowers.size(); i+=flower_stride){
      int dist = distance(flowers.at(i), p);

      if(dist == 0){
		      *did_land = true;
	    }

      if(dist != 0)
        cur_potential += (float)attraction_str*flower_stride/dist;
      else
        cur_potential += attraction_str*attraction_str;
    }

    return cur_potential;
  }

  void update_movement(int moveDist){
    move_dist = moveDist;
    set_threads();
    join_threads();
  }
};
