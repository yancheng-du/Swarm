#include <cassert>
#include <libfreenect.h>
#include <SDL.h>
#include <SDL_ttf.h>

#include "graphics.h"
#include "constants.h"

static SDL_Surface *graphics_create_SDL_surface_from_video_frame(const video_frame_t *video_frame);
static SDL_Surface *graphics_create_SDL_surface_from_depth_frame(const depth_frame_t *depth_frame);
static SDL_Surface *graphics_create_SDL_surface_from_edge_frame(const edge_frame_t *edge_frame);

SDL_Window *g_window= NULL;
SDL_Renderer *g_renderer= NULL;
TTF_Font *g_font= NULL;
static int g_frame_count= 0;
static uint64_t g_last_frame_time= 0;
static double g_frame_rate= k_fps;

bool graphics_initialize()
{
	bool success= false;

	if (SDL_CreateWindowAndRenderer(2*k_camera_width, 2*k_camera_height, 0, &g_window, &g_renderer)==0)
	{
		if (TTF_Init()==0)
		{
			g_font= TTF_OpenFont("res/monofonto.otf", 48);

			if (g_font)
			{
				g_frame_count= 0;
				g_last_frame_time= SDL_GetPerformanceCounter();
				g_frame_rate= k_fps;

				success= true;
			}
			else
			{
				SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "Couldn't open font: %s", TTF_GetError());
			}
		}
		else
		{
			SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "Couldn't initialize TTF: %s", TTF_GetError());
		}
	}
	else
	{
		SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "Couldn't create window and renderer: %s", SDL_GetError());
	}

	return success;
}

void graphics_dispose()
{
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

int graphics_render(const video_frame_t *video_frame, const depth_frame_t *depth_frame, const edge_frame_t *edge_frame, const swarm_t *swarm)
{
	if (g_renderer)
	{
		// render black background
		{
			SDL_SetRenderDrawColor(g_renderer, 0x00, 0x00, 0x00, 0x00);
			SDL_RenderClear(g_renderer);
		}

		// render video
		{
			SDL_Surface *video_surface= graphics_create_SDL_surface_from_video_frame(video_frame);

			if (video_surface)
			{
				SDL_Texture *video_texture= SDL_CreateTextureFromSurface(g_renderer, video_surface);
				SDL_FreeSurface(video_surface);

				if (video_texture)
				{
					SDL_Rect video_rect= {0, 0, k_camera_width, k_camera_height};

					SDL_RenderCopy(g_renderer, video_texture, NULL, &video_rect);
					SDL_DestroyTexture(video_texture);
				}
				else
				{
					SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "Couldn't create video texture: %s", SDL_GetError());
				}
			}
			else
			{
				SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "Couldn't create video surface: %s", SDL_GetError());
			}
		}

		// render depth
		{
			SDL_Surface *depth_surface= graphics_create_SDL_surface_from_depth_frame(depth_frame);

			if (depth_surface)
			{
				SDL_Texture *depth_texture= SDL_CreateTextureFromSurface(g_renderer, depth_surface);
				SDL_FreeSurface(depth_surface);

				if (depth_texture)
				{
					SDL_Rect depth_rect= {0, k_camera_height, k_camera_width, k_camera_height};

					SDL_RenderCopy(g_renderer, depth_texture, NULL, &depth_rect);
					SDL_DestroyTexture(depth_texture);
				}
				else
				{
					SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "Couldn't create depth texture: %s", SDL_GetError());
				}
			}
			else
			{
				SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "Couldn't create depth surface: %s", SDL_GetError());
			}
		}

		// render edges
		{
			SDL_Surface *edge_surface= graphics_create_SDL_surface_from_edge_frame(edge_frame);

			if (edge_surface)
			{
				SDL_Texture *edge_texture= SDL_CreateTextureFromSurface(g_renderer, edge_surface);
				SDL_FreeSurface(edge_surface);

				if (edge_texture)
				{
					SDL_Rect edge_rect= {k_camera_width, 0, k_camera_width, k_camera_height};

					SDL_RenderCopy(g_renderer, edge_texture, NULL, &edge_rect);
					SDL_DestroyTexture(edge_texture);
				}
				else
				{
					SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "Couldn't create edge texture: %s", SDL_GetError());
				}
			}
			else
			{
				SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "Couldn't create edge surface: %s", SDL_GetError());
			}
		}

		// render bees
		if (swarm)
		{
			SDL_Point points[k_bee_count];

			for (int bee_index= 0; bee_index<k_bee_count; ++bee_index)
			{
				points[bee_index].x= k_camera_width+swarm->bees[bee_index].p_x;
				points[bee_index].y= k_camera_height+swarm->bees[bee_index].p_y;
			}

			SDL_SetRenderDrawColor(g_renderer, 255, 255, 0, 255);
			SDL_RenderDrawPoints(g_renderer, points, k_bee_count);
		}

		// render frame rate
		if (g_font)
		{
			char frame_rate_string[8]= {0, 0, 0, 0, 0, 0, 0, 0};
			SDL_Color green= {0x0, 0xff, 0x0, 0xff};
			snprintf(frame_rate_string, sizeof(frame_rate_string)-1, "%.0f", g_frame_rate);
			SDL_Surface *text_surface= TTF_RenderText_Blended(g_font, frame_rate_string, green);

			if (text_surface)
			{
				SDL_Texture *text_texture= SDL_CreateTextureFromSurface(g_renderer, text_surface);
				SDL_FreeSurface(text_surface);

				if (text_texture)
				{

					SDL_Rect text_rect= {2*static_cast<int>(k_camera_width)-text_surface->w-8, 2*static_cast<int>(k_camera_height)-text_surface->h-8, text_surface->w, text_surface->h};

					SDL_RenderCopy(g_renderer, text_texture, NULL, &text_rect);
				}
				else
				{
					SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "Couldn't create text texture: %s", SDL_GetError());
				}
			}
			else
			{
				SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "Couldn't create text surface: %s", SDL_GetError());
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

static SDL_Surface *graphics_create_SDL_surface_from_video_frame(const video_frame_t *video_frame)
{
	const Uint32 k_red_mask= 0x000000ff;
	const Uint32 k_green_mask= 0x0000ff00;
	const Uint32 k_blue_mask= 0x00ff0000;
	const Uint32 k_alpha_mask= 0x00000000;

	return SDL_CreateRGBSurfaceFrom((void *)video_frame, k_camera_width, k_camera_height, 24, 3*k_camera_width, k_red_mask, k_green_mask, k_blue_mask, k_alpha_mask);
}

static SDL_Surface *graphics_create_SDL_surface_from_depth_frame(const depth_frame_t *depth_frame)
{
	SDL_Surface *surface= SDL_CreateRGBSurfaceWithFormat(0, k_camera_width, k_camera_height, 24, SDL_PIXELFORMAT_RGB24);

	if (surface)
	{
		const uint16_t *depth= (const uint16_t *)depth_frame;
		uint8_t *color= (uint8_t *)surface->pixels;

		assert(!SDL_MUSTLOCK(surface));
		assert(surface->pitch==3*k_camera_width);

		for (int y= 0; y<k_camera_height; ++y)
		{
			for (int x= 0; x<k_camera_width; ++x)
			{
				uint8_t depth8= UINT8_MAX-((*depth++)*UINT8_MAX)/FREENECT_DEPTH_MM_MAX_VALUE;

				*color++= depth8;
				*color++= depth8;
				*color++= depth8;
			}
		}
	}

	return surface;
}

static SDL_Surface *graphics_create_SDL_surface_from_edge_frame(const edge_frame_t *edge_frame)
{
	SDL_Surface *surface= SDL_CreateRGBSurfaceWithFormat(0, k_camera_width, k_camera_height, 24, SDL_PIXELFORMAT_RGB24);

	if (surface)
	{
		const uint8_t *edge= (const uint8_t *)edge_frame;
		uint8_t *color= (uint8_t *)surface->pixels;

		assert(!SDL_MUSTLOCK(surface));
		assert(surface->pitch==3*k_camera_width);

		for (int y= 0; y<k_camera_height; ++y)
		{
			for (int x= 0; x<k_camera_width; ++x)
			{
				uint8_t temp= *edge++;

				*color++= temp;
				*color++= temp;
				*color++= temp;
			}
		}
	}

	return surface;
}
