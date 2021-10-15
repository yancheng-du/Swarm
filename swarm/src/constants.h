#ifndef constants_h
#define constants_h

const int k_fps= 60;
const float k_dt= 1.0f/k_fps;

const int idle_checks_per_sec= 5;
const int seconds_before_idle= 10;
const float running_avg_alpha= 2.0f/(seconds_before_idle*60.0f/(k_fps/idle_checks_per_sec)+1.0f);

#endif /* constants_h */
