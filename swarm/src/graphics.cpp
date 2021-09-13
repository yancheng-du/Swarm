#include <cassert>
#include <SDL.h>

#include "graphics.h"

static SDL_Surface *graphics_create_SDL_surface_from_depth_frame(const depth_frame_t *depth_frame);

SDL_Window *g_window= NULL;
SDL_Renderer *g_renderer= NULL;
static int g_frame_count= 0;

bool graphics_initialize()
{
	bool success= false;

	if (SDL_CreateWindowAndRenderer(k_camera_width, 2*k_camera_height, 0, &g_window, &g_renderer)==0)
	{
		success= true;
	}
	else
	{
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create window and renderer: %s", SDL_GetError());
	}

	return success;
}

void graphics_dispose()
{
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

int graphics_render(const video_frame_t *video_frame, const depth_frame_t *depth_frame)
{
	if (g_renderer)
	{
		SDL_Surface *video_surface, *depth_surface;

		SDL_SetRenderDrawColor(g_renderer, 0x00, 0x00, 0x00, 0x00);
		SDL_RenderClear(g_renderer);

		video_surface= SDL_CreateRGBSurfaceFrom((void *)video_frame, k_camera_width, k_camera_height, 24, 3*k_camera_width, 0xff, 0xff00, 0xff0000, 0);

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
				SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create video texture: %s", SDL_GetError());
			}
		}
		else
		{
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create video surface: %s", SDL_GetError());
		}

		depth_surface= graphics_create_SDL_surface_from_depth_frame(depth_frame);

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
				SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create depth texture: %s", SDL_GetError());
			}
		}
		else
		{
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create depth surface: %s", SDL_GetError());
		}

		SDL_RenderPresent(g_renderer);
	}

	return g_frame_count++;
}

static SDL_Surface *graphics_create_SDL_surface_from_depth_frame(const depth_frame_t *depth_frame)
{
	SDL_Surface *surface= SDL_CreateRGBSurfaceWithFormat(0, k_camera_width, k_camera_height, 24, SDL_PIXELFORMAT_RGB24);

	assert(!SDL_MUSTLOCK(surface));

	if (surface)
	{
		const uint16_t *depth= (const uint16_t *)depth_frame;
		uint8_t *color= (uint8_t *)surface->pixels;

		assert(surface->pitch==3*k_camera_width);

		for (int y= 0; y<k_camera_height; ++y)
		{
			for (int x= 0; x<k_camera_width; ++x)
			{
				uint8_t depth8= 255-((*depth++)*255)/10000;

				*color++= depth8;
				*color++= depth8;
				*color++= depth8;
			}
		}
	}

	return surface;
}
