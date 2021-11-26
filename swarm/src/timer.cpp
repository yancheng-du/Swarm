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

void timer_t::start(double time)
{
	counter= SDL_GetPerformanceCounter() + static_cast<uint64_t>(time*frequency);
}

bool timer_t::running()
{
	return SDL_GetPerformanceCounter()<counter;
}
