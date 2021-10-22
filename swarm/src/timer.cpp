#include "timer.hpp"

timer_t::timer_t()
{
	frequency= static_cast<double>(SDL_GetPerformanceFrequency());
	reset();
}

void timer_t::reset()
{
	counter= SDL_GetPerformanceCounter();
}

bool timer_t::passed(double time)
{
	return (SDL_GetPerformanceCounter()-counter)/frequency>time;
}
