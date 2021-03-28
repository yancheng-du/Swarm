#include "BeeHandle.h"
#include <iostream>
#include <thread>

using namespace std;

float RandomFloat(float a, float b)
{
	float random = ((float)rand()) / (float)RAND_MAX;
	float diff = b - a;
	float r = random * diff;
	return a + r;
}

int rads2Dir(float rads){
	if ((3 * PI/8) > rads && rads >= (PI/8)) {
		return 1;
	} else if ((PI/8) > rads && rads >= (-1 * PI/8)) {
		return 2;
	} else if ((-1 * PI/8) > rads && rads >= (-3 * PI/8)) {
		return 3;
	} else if ((-3 * PI/8) > rads && rads >= (-5 * PI/8)) {
		return 4;
	} else if ((-5 * PI/8) > rads && rads >= (-7 * PI/8)) {
		return 5;
	} else if ((-7 * PI/8) > rads || rads >= (7 * PI/8)) {
		return 6;
	} else if ((7 * PI/8) > rads && rads >= (5 * PI/8)) {
		return 7;
	} else {
		return 0;
	}
}

static vector<vector<Attractor>> attractorMatrix;
static vector<vector<cv::Point>> attractorHistory;
static vector<cv::Point> staticPoints;

static void UpdateAttractorMatrix(int start_idx, int end_idx, int avgPercent, int storedFrames) {
	for (int P_idx = start_idx; P_idx < end_idx; P_idx++) {
		// If the position they are at has no other point and there is an attractor there with a good enough score
		int x = staticPoints[P_idx].x;
		int y = staticPoints[P_idx].y;
		int pointIdx = attractorMatrix[x][y].pointIdx;
		int score = attractorMatrix[x][y].score;

		if (pointIdx == -1 && score > (avgPercent * ((int)attractorHistory.size()))) {
			// Save the bee to that position, don't move the bee
			attractorMatrix[x][y].pointIdx = P_idx;
		}
	}
}

/*static void movePoint(int start_idx, int end_idx, int randomFactor, int stepSize, int xWidth, int yWidth) {
	for (int P_idx = start_idx; P_idx < end_idx; P_idx++) {
		if (attractorMatrix[staticPoints[P_idx].x][staticPoints[P_idx].y].pointIdx != P_idx) {
			float dist_x, dist_y;

			//move randomly
			dist_x = RandomFloat(-PI, PI);
			dist_y = RandomFloat(-PI, PI);

			float rads = atan2(dist_y, dist_x) + RandomFloat(-1 * randomFactor, randomFactor);

			int new_x, new_y;

			if (!(dist_x == 0 && dist_y == 0)) {
				new_x = staticPoints[P_idx].x + int(cos(rads) * stepSize);
				new_y = staticPoints[P_idx].y + int(sin(rads) * stepSize);

				if (new_x < 0 || new_x > xWidth) {
					staticPoints[P_idx].x = staticPoints[P_idx].x - int(cos(rads) * stepSize);
				}
				else {
					staticPoints[P_idx].x = new_x;
				}

				if (new_y < 0 || new_y > yWidth) {
					staticPoints[P_idx].y = staticPoints[P_idx].y - int(sin(rads) * stepSize);
				}
				else {
					staticPoints[P_idx].y = new_y;
				}
			}
		}
	}
}*/

static void removeLastFrame(int start_idx, int end_idx, double avgPercent) {
	for (int i = start_idx; i < end_idx; i++) {
		attractorMatrix[attractorHistory[0][i].x][attractorHistory[0][i].y].score = attractorMatrix[attractorHistory[0][i].x][attractorHistory[0][i].y].score - 1;
		if (attractorMatrix[attractorHistory[0][i].x][attractorHistory[0][i].y].score < (avgPercent * attractorHistory.size())){
			attractorMatrix[attractorHistory[0][i].x][attractorHistory[0][i].y].pointIdx = -1;
		}
	}
}

BeeHandle::BeeHandle(int xwidth, int ywidth, int stepsize, double randomfactor, int numthreads, int stored_frames, double avg_percent, int sound_divisor) {
	xWidth = xwidth;
	yWidth = ywidth;
	stepSize = stepsize;
	randomFactor = randomfactor;
	numThreads = numthreads;
	avgPercent = avg_percent;
	storedFrames = stored_frames;
	attractorMatrix = vector<vector<Attractor>>(xWidth, vector<Attractor>(yWidth, Attractor(0, 0)));
	soundDivisor = sound_divisor;
	for (unsigned int i = 0; i < attractorMatrix.size(); i++) {
		for (unsigned int j = 0; j < attractorMatrix[0].size(); j++) {
			attractorMatrix[i][j].x = i;
			attractorMatrix[i][j].y = j;
		}
	}
	attractorHistory.clear();
	staticPoints.clear();
	dirs.clear();
	landed.clear();
	sudo_landed.clear();
}

void BeeHandle::movePoints() {
	if(points.size() > dirs.size()){
		for(unsigned int i = dirs.size(); i < points.size(); i++){
			dirs.push_back(0);
		}
	}
	if(points.size()/soundDivisor > landed.size()){
		for(unsigned int i = landed.size(); i < points.size()/soundDivisor; i++){
			landed.push_back(0);
			sudo_landed.push_back(0);
		}
	}
	for (int P_idx = 0; P_idx < ((int)points.size()); P_idx++) {
		if (attractorMatrix[points[P_idx].x][points[P_idx].y].pointIdx == P_idx){
			if(P_idx < ((int)points.size())/soundDivisor){
				if(sudo_landed[P_idx] == 0){
					landed[P_idx] = 1;
					sudo_landed[P_idx] = 1;
				} else if (sudo_landed[P_idx] == 1){
					landed[P_idx] = 0;
					sudo_landed[P_idx] = 2;
				}
			}
		}
		else {
			if(P_idx < ((int)points.size())/soundDivisor){
				landed[P_idx] = 0;
				sudo_landed[P_idx] = 0;
			}

			float dist_x, dist_y;

			//move randomly
			dist_x = RandomFloat(-PI, PI);
			dist_y = RandomFloat(-PI, PI);

			float rads = atan2(dist_y, dist_x) + RandomFloat(-1 * randomFactor, randomFactor);

			int new_x, new_y;
			float x_change = 0.0;
			float y_change = 1.0;

			if (!(dist_x == 0 && dist_y == 0)) {
				new_x = points[P_idx].x + int(cos(rads) * stepSize);
				new_y = points[P_idx].y + int(sin(rads) * stepSize);

				if (new_x < 0 || new_x >= xWidth) {
					points[P_idx].x = points[P_idx].x - int(cos(rads) * stepSize);
					x_change = -1 * cos(rads);
				}
				else {
					points[P_idx].x = new_x;
					x_change = cos(rads);
				}

				if (new_y < 0 || new_y >= yWidth) {
					points[P_idx].y = points[P_idx].y - int(sin(rads) * stepSize);
					y_change = -1 * sin(rads);
				}
				else {
					points[P_idx].y = new_y;
					y_change = sin(rads);
				}
				dirs[P_idx] = rads2Dir(atan2(y_change, x_change));
			}
		}
	}
}

void BeeHandle::updatePoints() {
	vector<thread> threads_update;
	vector<thread> threads_move;

	int subSize = points.size() / this->numThreads;
	int subRem = points.size() % this->numThreads;

	staticPoints = points;

	Attractor a(0,0);

	for(int x = 0; x < xWidth; x++){
		for(int y = 0; y < yWidth; y++){
			a = attractorMatrix[x][y];
		}
	}

	for (int i = 0; i < this->numThreads; i++) {
		int start;
		int end;
		// If there are less than 3 candidates in a thread, there is no point of multithreading
		if (points.size() < ((unsigned int)numThreads)) {
			if (i < ((int)points.size())) {
				start = i;
				end = i + 1;
				//UpdateAttractorMatrix(points, start, end, avgPercent, storedFrames);
				threads_update.push_back(thread(UpdateAttractorMatrix, start, end, avgPercent, storedFrames));
			}
		}
		// Assign a subvector of idx_array to a thread
		else if (i < subRem) {
			//if (i < 1) std::cout << "case 2" << std::endl;
			start = i * (subSize + 1);
			end = (i + 1) * (subSize + 1);
			//UpdateAttractorMatrix(points, start, end, avgPercent, storedFrames);
			threads_update.push_back(thread(UpdateAttractorMatrix, start, end, avgPercent, storedFrames));
		}
		// Assign a subvector of idx_array to a thread
		else {
			//if (i < 1) std::cout << "case 3" << std::endl;
			start = subRem * (subSize + 1) + (i - subRem) * (subSize);
			end = subRem * (subSize + 1) + (i - subRem + 1) * (subSize);
			//UpdateAttractorMatrix(points, start, end, avgPercent, storedFrames);
			threads_update.push_back(thread(UpdateAttractorMatrix, start, end, avgPercent, storedFrames));
		}
	}

	for (unsigned int j = 0; j < threads_update.size(); j++) {
		threads_update[j].join();
	}

	threads_update.clear();

	/*
	for (int i = 0; i < this->numThreads; i++) {
		int start;
		int end;
		// If there are less than 3 candidates in a thread, there is no point of multithreading
		if (points.size() < numThreads) {
			if (i < points.size()) {
				start = i;
				end = i + 1;
				//UpdateAttractorMatrix(points, start, end, avgPercent, storedFrames);
				threads_move.push_back(thread(movePoint, start, end, randomFactor, stepSize, xWidth, yWidth));
			}
		}
		// Assign a subvector of idx_array to a thread
		else if (i < subRem) {
			//if (i < 1) std::cout << "case 2" << std::endl;
			start = i * (subSize + 1);
			end = (i + 1) * (subSize + 1);
			//UpdateAttractorMatrix(points, start, end, avgPercent, storedFrames);
			threads_move.push_back(thread(movePoint, start, end, randomFactor, stepSize, xWidth, yWidth));
		}
		// Assign a subvector of idx_array to a thread
		else {
			//if (i < 1) std::cout << "case 3" << std::endl;
			start = subRem * (subSize + 1) + (i - subRem) * (subSize);
			end = subRem * (subSize + 1) + (i - subRem + 1) * (subSize);
			//UpdateAttractorMatrix(points, start, end, avgPercent, storedFrames);
			threads_move.push_back(thread(movePoint, start, end, randomFactor, stepSize, xWidth, yWidth));
		}
	}


	for (int j = 0; j < threads_move.size(); j++) {
		threads_move[j].join();
	}

	threads_move.clear();
	*/

	points = staticPoints;

	movePoints();
}

vector<int> BeeHandle::get_dirs(){
	return dirs;
}

vector<int> BeeHandle::get_landed(){
	return landed;
}

void BeeHandle::addAttractorsAvg(vector<cv::Point> new_attractors) {

	vector<vector<int>> temp(xWidth, vector<int> (yWidth, 0));

	// Remove duplicates
	// Cannot be done in parallel - O(f)
	for (int i = new_attractors.size()-1; i >= 0; i--) {
		// If a contour in that position already exists, remove
		/*cout << "new_attractors[i].x" << new_attractors[i].x << std::endl;
		cout << "xWidth" << xWidth << std::endl;
		cout << "new_attractors[i].y" << new_attractors[i].y << std::endl;
		cout << "yWidth" << yWidth << std::endl << std::endl;*/
		if (temp[new_attractors[i].x][new_attractors[i].y] == 1) {
			new_attractors.erase(new_attractors.begin() + i);
		}
		// If a contour in that position doesn't exist, increment attractorMatrix in that position and add reference to the position to attractorRefs
		else {
			temp[new_attractors[i].x][new_attractors[i].y] = 1;
			attractorMatrix[new_attractors[i].x][new_attractors[i].y].score = attractorMatrix[new_attractors[i].x][new_attractors[i].y].score + 1;
		}
	}

	//Now that dups are removed, add to history
	attractorHistory.push_back(new_attractors);

	// Remove expired frame
	// Can be done in parallel (no duplicates) - O(f/t)
	/*
	if (attractorHistory.size() > storedFrames){
		for (int i = 0; i < attractorHistory[0].size(); i++) {
			attractorMatrix[attractorHistory[0][i].x][attractorHistory[0][i].y].score = attractorMatrix[attractorHistory[0][i].x][attractorHistory[0][i].y].score - 1;
		}
		attractorHistory.erase(attractorHistory.begin());
	}*/


	if (((int)attractorHistory.size()) > storedFrames) {
		vector<thread> threads;
		int subSize = attractorHistory[0].size() / this->numThreads;
		int subRem = attractorHistory[0].size() % this->numThreads;

		for (int i = 0; i < this->numThreads; i++) {
			int start;
			int end;
			// If there are less than 3 candidates in a thread, there is no point of multithreading
			if (((int)attractorHistory[0].size()) < numThreads) {
				if (i < ((int)attractorHistory[0].size())) {
					start = i;
					end = i + 1;
					//UpdateAttractorMatrix(points, start, end, avgPercent, storedFrames);
					threads.push_back(thread(removeLastFrame, start, end, avgPercent));
				}
			}
			// Assign a subvector of idx_array to a thread
			else if (i < subRem) {
				//if (i < 1) std::cout << "case 2" << std::endl;
				start = i * (subSize + 1);
				end = (i + 1) * (subSize + 1);
				//UpdateAttractorMatrix(points, start, end, avgPercent, storedFrames);
				threads.push_back(thread(removeLastFrame, start, end, avgPercent));
			}
			// Assign a subvector of idx_array to a thread
			else {
				//if (i < 1) std::cout << "case 3" << std::endl;
				start = subRem * (subSize + 1) + (i - subRem) * (subSize);
				end = subRem * (subSize + 1) + (i - subRem + 1) * (subSize);
				//UpdateAttractorMatrix(points, start, end, avgPercent, storedFrames);
				threads.push_back(thread(removeLastFrame, start, end, avgPercent));
			}
		}

		for (int j = 0; j < ((int)threads.size()); j++) {
			threads[j].join();
		}

		threads.clear();
		attractorHistory.erase(attractorHistory.begin());
	}
}
