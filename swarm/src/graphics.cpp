#include <cassert>
#include <cstdio>

#include <libfreenect.h>
#include <SDL.h>
#include <SDL_ttf.h>

#include "graphics.hpp"
#include "constants.hpp"
#include <iostream>

const int k_window_width= 540;
const int k_window_height= 960;

// assume 4:3 camera aspect ratio
const int k_scaled_camera_width= k_window_height/3;
const int k_scaled_camera_height= k_window_height/4;
const int k_scaled_clip_width= k_window_width*k_scaled_camera_height/k_window_height;

const SDL_Rect k_video_rect= {(k_window_width-k_scaled_camera_width)/2, 0, k_scaled_camera_width, k_scaled_camera_height};
const SDL_Rect k_video_clip_rect= {k_video_rect.x+(k_scaled_camera_width-k_scaled_clip_width)/2, 0, k_scaled_clip_width, k_scaled_camera_height};
const SDL_Rect k_depth_rect= {(k_window_width-k_scaled_camera_width)/2, k_scaled_camera_height, k_scaled_camera_width, k_scaled_camera_height};
const SDL_Rect k_depth_clip_rect= {k_depth_rect.x+(k_scaled_camera_width-k_scaled_clip_width)/2, k_scaled_camera_height, k_scaled_clip_width, k_scaled_camera_height};
const SDL_Rect k_edge_rect= {0, k_window_height/2, k_window_width/2, k_window_height/2};
const SDL_Rect k_swarm_rect= {k_window_width/2, k_window_height/2, k_window_width/2, k_window_height/2};

static SDL_Surface *graphics_create_SDL_surface_from_video_frame(const cv::Mat3b *video_frame);
static SDL_Surface *graphics_create_SDL_surface_from_depth_frame(const cv::Mat1w *depth_frame);
static SDL_Surface *graphics_create_SDL_surface_from_edge_frame(const cv::Mat1b *edge_frame);

static SDL_Texture* LoadTexture(std::string filePath);



SDL_Window *g_window= NULL;
SDL_Renderer *g_renderer= NULL;
TTF_Font *g_font= NULL;

//sprite texture
SDL_Texture* bee_sprite_texture = NULL;

static int g_frame_count= 0;
static uint64_t g_last_frame_time= 0;
static double g_frame_rate= k_fps;

bool graphics_initialize()
{
	bool success= false;

	if (SDL_CreateWindowAndRenderer(k_window_width, k_window_height, 0, &g_window, &g_renderer)==0)
	{
		if (TTF_Init()==0)
		{
			g_font= TTF_OpenFont("res/monofonto.otf", 48);

			if (g_font)
			{
				//load sprite texture
				bee_sprite_texture = LoadTexture("res/64_Fly_Sheet.bmp");



				g_frame_count= 0;
				g_last_frame_time= SDL_GetPerformanceCounter();
				g_frame_rate= k_fps;

				success= true;
			}
			else
			{
				SDL_LogError(SDL_LOG_CATEGORY_RENDER, "Couldn't open font: %s", TTF_GetError());
			}
		}
		else
		{
			SDL_LogError(SDL_LOG_CATEGORY_RENDER, "Couldn't initialize TTF: %s", TTF_GetError());
		}
	}
	else
	{
		SDL_LogError(SDL_LOG_CATEGORY_RENDER, "Couldn't create window and renderer: %s", SDL_GetError());
	}

	return success;
}

void graphics_dispose()
{
	//free sprite tetxure
	SDL_DestroyTexture(bee_sprite_texture);

	if (g_font)
	{
		TTF_CloseFont(g_font);
		g_font= NULL;
	}

	if (TTF_WasInit())
	{
		TTF_Quit();
	}

	if (g_renderer)
	{
		SDL_DestroyRenderer(g_renderer);
		g_renderer= NULL;
	}

	if (g_window)
	{
		SDL_DestroyWindow(g_window);
		g_window= NULL;
	}
}

int graphics_render(const swarm_t *swarm, bool fps, bool debug, const cv::Mat3b *video_frame, const cv::Mat1w *depth_frame, const cv::Mat1b *edge_frame)
{
	if (g_renderer)
	{
		// render clear
		{
			SDL_SetRenderDrawColor(g_renderer, 0x00, 0x00, 0x00, 0x00);
			SDL_RenderClear(g_renderer);
		}

		// render video
		if (debug)
		{
			SDL_Surface *video_surface= graphics_create_SDL_surface_from_video_frame(video_frame);

			if (video_surface)
			{
				SDL_Texture *video_texture= SDL_CreateTextureFromSurface(g_renderer, video_surface);
				SDL_FreeSurface(video_surface);

				if (video_texture)
				{
					SDL_RenderCopy(g_renderer, video_texture, NULL, &k_video_rect);
					SDL_DestroyTexture(video_texture);
					SDL_RenderDrawRect(g_renderer, &k_video_clip_rect);
				}
				else
				{
					SDL_LogError(SDL_LOG_CATEGORY_RENDER, "Couldn't create video texture: %s", SDL_GetError());
				}
			}
			else
			{
				SDL_LogError(SDL_LOG_CATEGORY_RENDER, "Couldn't create video surface: %s", SDL_GetError());
			}
		}

		// render depth
		if (debug)
		{
			SDL_Surface *depth_surface= graphics_create_SDL_surface_from_depth_frame(depth_frame);

			if (depth_surface)
			{
				SDL_Texture *depth_texture= SDL_CreateTextureFromSurface(g_renderer, depth_surface);
				SDL_FreeSurface(depth_surface);

				if (depth_texture)
				{
					SDL_RenderCopy(g_renderer, depth_texture, NULL, &k_depth_rect);
					SDL_DestroyTexture(depth_texture);
					SDL_RenderDrawRect(g_renderer, &k_depth_clip_rect);
				}
				else
				{
					SDL_LogError(SDL_LOG_CATEGORY_RENDER, "Couldn't create depth texture: %s", SDL_GetError());
				}
			}
			else
			{
				SDL_LogError(SDL_LOG_CATEGORY_RENDER, "Couldn't create depth surface: %s", SDL_GetError());
			}
		}

		// render edges
		if (debug)
		{
			SDL_Surface *edge_surface= graphics_create_SDL_surface_from_edge_frame(edge_frame);

			if (edge_surface)
			{
				SDL_Texture *edge_texture= SDL_CreateTextureFromSurface(g_renderer, edge_surface);
				SDL_FreeSurface(edge_surface);

				if (edge_texture)
				{
					SDL_RenderCopy(g_renderer, edge_texture, NULL, &k_edge_rect);
					SDL_DestroyTexture(edge_texture);
				}
				else
				{
					SDL_LogError(SDL_LOG_CATEGORY_RENDER, "Couldn't create edge texture: %s", SDL_GetError());
				}
			}
			else
			{
				SDL_LogError(SDL_LOG_CATEGORY_RENDER, "Couldn't create edge surface: %s", SDL_GetError());
			}
		}

		// render bees
		if (swarm)
		{
			// $TODO replace with sprite rendering!

			int x0= debug ? k_swarm_rect.x : 0;
			int y0= debug ? k_swarm_rect.y : 0;
			int dx= debug ? k_swarm_rect.w : k_window_width;
			int dy= debug ? k_swarm_rect.h : k_window_height;

			for (int state= bee_t::state_t::_idle; state<bee_t::state_t::k_state_count; ++state)
			{

				for (int bee_index= 0; bee_index<k_bee_count; ++bee_index)
				{
					const bee_t *bee= &swarm->bees[bee_index];

					if (bee->state==bee_t::state_t::_flying)
					{

						SDL_FRect rect;
						rect.x= x0 + static_cast<int>(bee->x / k_simulation_width * dx);
						rect.y = y0 + static_cast<int>(bee->y / k_simulation_height * dy);
						rect.w = 16;
						rect.h = 16;
						SDL_RenderCopyExF(g_renderer, bee_sprite_texture, &bee->b_rect, &rect, (((bee->facing * 180)/3.14) + 90), 0, SDL_FLIP_NONE);

					}
				}

				switch (state)
				{
					case bee_t::state_t::_idle: 	SDL_SetRenderDrawColor(g_renderer, 255, 127, 0, 255); break;
					case bee_t::state_t::_crawling: SDL_SetRenderDrawColor(g_renderer, 255, 191, 0, 255); break;
					case bee_t::state_t::_flying: 	SDL_SetRenderDrawColor(g_renderer, 255, 255, 0, 255); break;
				}

			}
		}

		// render frame rate
		if (fps && g_font)
		{	
			char frame_rate_string[4];
			SDL_Color green= {0x0, 0xff, 0x0, 0xff};
			snprintf(frame_rate_string, sizeof(frame_rate_string), "%3.0f", g_frame_rate);
			SDL_Surface *text_surface= TTF_RenderText_Blended(g_font, frame_rate_string, green);

			if (text_surface)
			{	
				SDL_Texture *text_texture= SDL_CreateTextureFromSurface(g_renderer, text_surface);
				SDL_FreeSurface(text_surface);

				if (text_texture)
				{	
					SDL_Rect text_rect= {k_window_width-text_surface->w-8, 8, text_surface->w, text_surface->h};
					SDL_RenderCopy(g_renderer, text_texture, NULL, &text_rect);
					SDL_DestroyTexture(text_texture);
				}
				else
				{
					SDL_LogError(SDL_LOG_CATEGORY_RENDER, "Couldn't create text texture: %s", SDL_GetError());
				}
			}
			else
			{
				SDL_LogError(SDL_LOG_CATEGORY_RENDER, "Couldn't create text surface: %s", SDL_GetError());
			}
		}

		SDL_RenderPresent(g_renderer);

		// update fps
		{
			uint64_t current_frame_time= SDL_GetPerformanceCounter();

			g_frame_rate= static_cast<double>(SDL_GetPerformanceFrequency())/(current_frame_time-g_last_frame_time);
			g_last_frame_time= current_frame_time;
		}
	}

	return g_frame_count++;
}

static SDL_Surface *graphics_create_SDL_surface_from_video_frame(const cv::Mat3b *video_frame)
{
	const Uint32 k_red_mask= 0x000000ff;
	const Uint32 k_green_mask= 0x0000ff00;
	const Uint32 k_blue_mask= 0x00ff0000;
	const Uint32 k_alpha_mask= 0x00000000;

	assert(video_frame->isContinuous());

	return SDL_CreateRGBSurfaceFrom(video_frame->data, video_frame->cols, video_frame->rows, 24, 3*video_frame->cols, k_red_mask, k_green_mask, k_blue_mask, k_alpha_mask);
}

static SDL_Surface *graphics_create_SDL_surface_from_depth_frame(const cv::Mat1w *depth_frame)
{
	SDL_Surface *surface= SDL_CreateRGBSurfaceWithFormat(0, depth_frame->cols, depth_frame->rows, 24, SDL_PIXELFORMAT_RGB24);

	if (surface)
	{
		const uint16_t *depth= depth_frame->ptr<uint16_t>();
		uint8_t *color_row= (uint8_t *)surface->pixels;

		assert(depth_frame->isContinuous());
		assert(!SDL_MUSTLOCK(surface));

		for (int y= 0; y<surface->h; ++y)
		{
			uint8_t *color= color_row;

			for (int x= 0; x<surface->w; ++x)
			{
				uint8_t depth8= UINT8_MAX-((*depth++)*UINT8_MAX)/FREENECT_DEPTH_MM_MAX_VALUE;

				*color++= depth8;
				*color++= depth8;
				*color++= depth8;
			}

			color_row+= surface->pitch;
		}
	}

	return surface;
}

static SDL_Surface *graphics_create_SDL_surface_from_edge_frame(const cv::Mat1b *edge_frame)
{
	SDL_Surface *surface= SDL_CreateRGBSurfaceWithFormat(0, edge_frame->cols, edge_frame->rows, 24, SDL_PIXELFORMAT_RGB24);

	if (surface)
	{
		const uint8_t *edge= edge_frame->ptr<uint8_t>();
		uint8_t *color_row= (uint8_t *)surface->pixels;

		assert(edge_frame->isContinuous());
		assert(!SDL_MUSTLOCK(surface));

		for (int y= 0; y<surface->h; ++y)
		{
			uint8_t *color= color_row;

			for (int x= 0; x<surface->w; ++x)
			{
				uint8_t temp= *edge++;

				*color++= temp;
				*color++= temp;
				*color++= temp;
			}

			color_row+= surface->pitch;
		}
	}

	return surface;
}

static SDL_Texture* LoadTexture(std::string filePath)
{
	SDL_Texture* texture = nullptr;
	SDL_Surface* surface = SDL_LoadBMP(filePath.c_str());
	if (surface == NULL)
		std::cout << "ERROR LOADING BMP" << std::endl;
	else
	{
		texture = SDL_CreateTextureFromSurface(g_renderer, surface);
		if (texture == NULL)
			std::cout << "ERROR CREATING TEXTURE FROM SURFACE" << std::endl;
	}

	SDL_FreeSurface(surface);
	return texture; 
}