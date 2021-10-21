#include <cassert>
#include <cstdio>

#include <libfreenect.h>
#include <SDL.h>
#include <SDL_ttf.h>

#include "graphics.hpp"
#include "constants.hpp"

const int k_window_width= 540;
const int k_window_height= 960;

// assume 4:3 camera aspect ratio so use the 9:16 center portion of it
const int k_scaled_camera_width= k_window_height/3;
const int k_scaled_camera_height= k_window_height/4;
const int k_scaled_clip_width= k_window_width*k_scaled_camera_height/k_window_height;

// video frame is top half of the top half of the window
const SDL_Rect k_video_rect= {(k_window_width-k_scaled_camera_width)/2, 0, k_scaled_camera_width, k_scaled_camera_height};
const SDL_Rect k_video_clip_rect= {k_video_rect.x+(k_scaled_camera_width-k_scaled_clip_width)/2, 0, k_scaled_clip_width, k_scaled_camera_height};

// depth frame is bottom half of the top half of the window
const SDL_Rect k_depth_rect= {(k_window_width-k_scaled_camera_width)/2, k_scaled_camera_height, k_scaled_camera_width, k_scaled_camera_height};
const SDL_Rect k_depth_clip_rect= {k_depth_rect.x+(k_scaled_camera_width-k_scaled_clip_width)/2, k_scaled_camera_height, k_scaled_clip_width, k_scaled_camera_height};

// edge frame is the lower left quarter of the window
const SDL_Rect k_edge_rect= {0, k_window_height/2, k_window_width/2, k_window_height/2};

// swarm is the lower right quarter of the window
const SDL_Rect k_swarm_rect= {k_window_width/2, k_window_height/2, k_window_width/2, k_window_height/2};

static SDL_Texture *graphics_create_texture_from_image_file(const char *filePath);
static SDL_Texture *graphics_create_texture_from_video_frame(const cv::Mat3b &video_frame);
static SDL_Texture *graphics_create_texture_from_depth_frame(const cv::Mat1w &depth_frame);
static SDL_Texture *graphics_create_texture_from_edge_frame(const cv::Mat1b &edge_frame);
static SDL_Texture *graphics_create_texture_from_string(const char *string, const SDL_Color &color, int &height, int &width);

SDL_Window *g_window= NULL;
SDL_Renderer *g_renderer= NULL;
TTF_Font *g_font= NULL;
SDL_Texture *g_bee_idle_texture= NULL;
SDL_Texture *g_bee_crawl_texture= NULL;
SDL_Texture *g_bee_fly_texture= NULL;

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
				g_bee_idle_texture= graphics_create_texture_from_image_file("res/64_Idle_Sheet.bmp");
				g_bee_crawl_texture= graphics_create_texture_from_image_file("res/64_Crawl_Sheet.bmp");
				g_bee_fly_texture= graphics_create_texture_from_image_file("res/64_Fly_Sheet.bmp");

				if (g_bee_idle_texture &&
					g_bee_crawl_texture &&
					g_bee_fly_texture)
				{
					g_frame_count= 0;
					g_last_frame_time= SDL_GetPerformanceCounter();
					g_frame_rate= k_fps;

					success= true;
				}
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
	if (g_bee_fly_texture)
	{
		SDL_DestroyTexture(g_bee_fly_texture);
		g_bee_fly_texture= NULL;
	}

	if (g_bee_crawl_texture)
	{
		SDL_DestroyTexture(g_bee_crawl_texture);
		g_bee_crawl_texture= NULL;
	}

	if (g_bee_idle_texture)
	{
		SDL_DestroyTexture(g_bee_idle_texture);
		g_bee_idle_texture= NULL;
	}

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

int graphics_render(const swarm_t &swarm, bool debug, const cv::Mat3b &video_frame, const cv::Mat1w &depth_frame, const cv::Mat1b &edge_frame, bool fps)
{
	if (g_renderer)
	{
		// clear background
		if (debug)
		{
			SDL_SetRenderDrawColor(g_renderer, 0x00, 0x00, 0x00, 0x00);
		}
		else
		{
			SDL_SetRenderDrawColor(g_renderer, 0xff, 0xff, 0xff, 0x00);
		}
		SDL_RenderClear(g_renderer);

		// render bees
		{
			float x0= static_cast<float>(debug ? k_swarm_rect.x : 0);
			float y0= static_cast<float>(debug ? k_swarm_rect.y : 0);
			float dx= static_cast<float>(debug ? k_swarm_rect.w : k_window_width)/k_simulation_width;
			float dy= static_cast<float>(debug ? k_swarm_rect.h : k_window_height)/k_simulation_height;

			if (debug)
			{
				for (int state= bee_t::state_t::_idle; state<bee_t::state_t::k_state_count; ++state)
				{
					SDL_FPoint points[k_bee_count];
					int point_count= 0;

					for (int bee_index= 0; bee_index<k_bee_count; ++bee_index)
					{
						const bee_t *bee= &swarm.bees[bee_index];

						if (bee->state==state)
						{
							SDL_FPoint *point= &points[point_count++];

							point->x= x0 + bee->x*dx;
							point->y= y0 + bee->y*dy;
						}
					}

					switch (state)
					{
						case bee_t::state_t::_idle:     SDL_SetRenderDrawColor(g_renderer, 255, 127, 0, 255); break;
						case bee_t::state_t::_crawling: SDL_SetRenderDrawColor(g_renderer, 255, 191, 0, 255); break;
						default:                        SDL_SetRenderDrawColor(g_renderer, 255, 255, 0, 255); break;
					}

					SDL_RenderDrawPointsF(g_renderer, points, point_count);
				}
			}
			else
			{
				SDL_FRect rect;

				rect.w= 2*k_bee_radius*dx;
				rect.h= 2*k_bee_radius*dy;

				for (int state= bee_t::state_t::_idle; state<bee_t::state_t::k_state_count; ++state)
				{
					for (int bee_index= 0; bee_index<k_bee_count; ++bee_index)
					{
						const bee_t *bee= &swarm.bees[bee_index];

						if (bee->state==state)
						{
							rect.x= x0 + (bee->x-k_bee_radius)*dx;
							rect.y= y0 + (bee->y-k_bee_radius)*dy;

							switch (state)
							{
								case bee_t::state_t::_idle:     SDL_RenderCopyExF(g_renderer, g_bee_idle_texture,  &bee->b_idle_rect,  &rect, 57.2957795131*bee->facing+90.0, NULL, SDL_FLIP_NONE); break;
								case bee_t::state_t::_crawling: SDL_RenderCopyExF(g_renderer, g_bee_crawl_texture, &bee->b_crawl_rect, &rect, 57.2957795131*bee->facing+90.0, NULL, SDL_FLIP_NONE); break;
								default:                        SDL_RenderCopyExF(g_renderer, g_bee_fly_texture,   &bee->b_fly_rect,   &rect, 57.2957795131*bee->facing+90.0, NULL, SDL_FLIP_NONE); break;
							}
						}
					}
				}
			}
		}

		// render video
		if (debug)
		{
			SDL_Texture *video_texture= graphics_create_texture_from_video_frame(video_frame);

			if (video_texture)
			{
				SDL_RenderCopy(g_renderer, video_texture, NULL, &k_video_rect);
				SDL_DestroyTexture(video_texture);
				SDL_RenderDrawRect(g_renderer, &k_video_clip_rect);
			}
		}

		// render depth
		if (debug)
		{
			SDL_Texture *depth_texture= graphics_create_texture_from_depth_frame(depth_frame);

			if (depth_texture)
			{
				SDL_RenderCopy(g_renderer, depth_texture, NULL, &k_depth_rect);
				SDL_DestroyTexture(depth_texture);
				SDL_RenderDrawRect(g_renderer, &k_depth_clip_rect);
			}
		}

		// render edges
		if (debug)
		{
			SDL_Texture *edge_texture= graphics_create_texture_from_edge_frame(edge_frame);

			if (edge_texture)
			{
				SDL_RenderCopy(g_renderer, edge_texture, NULL, &k_edge_rect);
				SDL_DestroyTexture(edge_texture);
			}
		}

		// render frame rate
		if (fps)
		{
			char frame_rate_string[4];
			snprintf(frame_rate_string, sizeof(frame_rate_string), "%3.0f", g_frame_rate);
			SDL_Color color= {0x0, 0xff, 0x0, 0xff};
			int height, width;
			SDL_Texture *fps_texture= graphics_create_texture_from_string(frame_rate_string, color, height, width);

			if (fps_texture)
			{
				SDL_Rect text_rect= {k_window_width-width-8, 8, width, height};
				SDL_RenderCopy(g_renderer, fps_texture, NULL, &text_rect);
				SDL_DestroyTexture(fps_texture);
			}
		}

		// present frame
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

static SDL_Texture* graphics_create_texture_from_image_file(const char *filepath)
{
	SDL_Texture *texture= NULL;
	SDL_Surface *surface= SDL_LoadBMP(filepath);

	if (surface)
	{
		texture= SDL_CreateTextureFromSurface(g_renderer, surface);
		SDL_FreeSurface(surface);

		if (texture)
		{
			// success!
		}
		else
		{
			SDL_LogError(SDL_LOG_CATEGORY_RENDER, "Couldn't create texture from surface for '%s': %s", filepath, SDL_GetError());
		}
	}
	else
	{
		SDL_LogError(SDL_LOG_CATEGORY_RENDER, "Couldn't load '%s': %s", filepath, SDL_GetError());
	}

	return texture;
}

static SDL_Texture *graphics_create_texture_from_video_frame(const cv::Mat3b &video_frame)
{
	SDL_Texture *texture= NULL;

	assert(video_frame.isContinuous());
	{
		const Uint32 k_red_mask= 0x000000ff;
		const Uint32 k_green_mask= 0x0000ff00;
		const Uint32 k_blue_mask= 0x00ff0000;
		const Uint32 k_alpha_mask= 0x00000000;
		SDL_Surface *surface= SDL_CreateRGBSurfaceFrom(video_frame.data, video_frame.cols, video_frame.rows, 24, 3*video_frame.cols, k_red_mask, k_green_mask, k_blue_mask, k_alpha_mask);

		if (surface)
		{
			texture= SDL_CreateTextureFromSurface(g_renderer, surface);
			SDL_FreeSurface(surface);

			if (texture)
			{
				// success!
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

	return texture;
}

static SDL_Texture *graphics_create_texture_from_depth_frame(const cv::Mat1w &depth_frame)
{
	SDL_Texture *texture= NULL;
	SDL_Surface *surface= SDL_CreateRGBSurfaceWithFormat(0, depth_frame.cols, depth_frame.rows, 24, SDL_PIXELFORMAT_RGB24);

	if (surface)
	{
		assert(depth_frame.isContinuous());
		assert(!SDL_MUSTLOCK(surface));
		{
			const uint16_t *depth= depth_frame.ptr<uint16_t>();
			uint8_t *color_row= (uint8_t *)surface->pixels;

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

		texture= SDL_CreateTextureFromSurface(g_renderer, surface);
		SDL_FreeSurface(surface);

		if (texture)
		{
			// success!
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

	return texture;
}

static SDL_Texture *graphics_create_texture_from_edge_frame(const cv::Mat1b &edge_frame)
{
	SDL_Texture *texture= NULL;
	SDL_Surface *surface= SDL_CreateRGBSurfaceWithFormat(0, edge_frame.cols, edge_frame.rows, 24, SDL_PIXELFORMAT_RGB24);

	if (surface)
	{
		assert(edge_frame.isContinuous());
		assert(!SDL_MUSTLOCK(surface));
		{
			const uint8_t *edge= edge_frame.ptr<uint8_t>();
			uint8_t *color_row= (uint8_t *)surface->pixels;

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

		texture= SDL_CreateTextureFromSurface(g_renderer, surface);
		SDL_FreeSurface(surface);

		if (texture)
		{
			// success!
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

	return texture;
}

static SDL_Texture *graphics_create_texture_from_string(const char *string, const SDL_Color &color, int &height, int &width)
{
	SDL_Texture *texture= NULL;

	height= 0;
	width= 0;

	if (g_font)
	{
		SDL_Surface *surface= TTF_RenderText_Blended(g_font, string, color);

		if (surface)
		{
			int surface_height= surface->h;
			int surface_width= surface->w;

			texture= SDL_CreateTextureFromSurface(g_renderer, surface);
			SDL_FreeSurface(surface);

			if (texture)
			{
				height= surface_height;
				width= surface_width;

				// success!
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

	return texture;
}
