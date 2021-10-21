#ifndef timer_hpp
#define timer_hpp

#include <SDL_timer.h>

class timer_t
{
public:
	timer_t();

	void reset();
	bool passed(double time);

private:
	double frequency;
	uint64_t counter;
};

#endif /* timer_hpp */
