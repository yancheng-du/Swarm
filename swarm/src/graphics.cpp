#include <SDL.h>

#include "graphics.h"

SDL_Window *g_window= NULL;
SDL_Renderer *g_renderer= NULL;
static int g_frame_count= 0;

bool graphics_initialize()
{
	bool success= false;

	if (SDL_CreateWindowAndRenderer(k_camera_width, k_camera_height, 0, &g_window, &g_renderer)==0)
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
		SDL_Surface *video_surface;

		SDL_SetRenderDrawColor(g_renderer, 0x00, 0x00, 0x00, 0x00);
		SDL_RenderClear(g_renderer);

		video_surface= SDL_CreateRGBSurfaceFrom((void *)video_frame, k_camera_width, k_camera_height, 24, 3*k_camera_width, 0xff, 0xff00, 0xff0000, 0);

		if (video_surface)
		{
			SDL_Texture *video_texture= SDL_CreateTextureFromSurface(g_renderer, video_surface);
			SDL_FreeSurface(video_surface);

			if (video_texture)
			{
				SDL_RenderCopy(g_renderer, video_texture, NULL, NULL);
				SDL_DestroyTexture(video_texture);
			}
			else
			{
				SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create texture from surface: %s", SDL_GetError());
			}
		}
		else
		{
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create surface: %s", SDL_GetError());
		}

		SDL_RenderPresent(g_renderer);
	}

	return g_frame_count++;
}
