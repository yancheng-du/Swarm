#include <SDL.h>

#include "graphics.h"

SDL_Window *g_window= NULL;
SDL_Renderer *g_renderer= NULL;
SDL_Texture *g_texture= NULL;
static int g_frame_count= 0;

bool graphics_initialize()
{
	bool success= false;

	if (SDL_CreateWindowAndRenderer(640, 360, SDL_WINDOW_RESIZABLE, &g_window, &g_renderer)==0)
	{
		SDL_Surface *surface= SDL_LoadBMP("res/carnage.bmp");

		if (surface)
		{
			g_texture= SDL_CreateTextureFromSurface(g_renderer, surface);

			SDL_FreeSurface(surface);

			if (g_texture)
			{
				success= true;
			}
			else
			{
				SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create texture from surface: %s", SDL_GetError());
			}
		}
		else
		{
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create surface from image: %s", SDL_GetError());
		}
	}
	else
	{
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create window and renderer: %s", SDL_GetError());
	}

	return success;
}

void graphics_dispose()
{
	if (g_texture)
	{
		SDL_DestroyTexture(g_texture);
		g_texture= NULL;
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

int graphics_render()
{
	if (g_renderer)
	{
		SDL_SetRenderDrawColor(g_renderer, 0x00, 0x00, 0x00, 0x00);
		SDL_RenderClear(g_renderer);

		if (g_texture)
		{
			SDL_RenderCopy(g_renderer, g_texture, NULL, NULL);
		}

		SDL_RenderPresent(g_renderer);
	}

	return g_frame_count++;
}
