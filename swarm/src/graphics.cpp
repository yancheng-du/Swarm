#include <cassert>
#include <cstdio>

#include <libfreenect.h>
#include <SDL.h>
#include <SDL_ttf.h>

#include "constants.hpp"
#include "graphics.hpp"

const int k_window_width= k_simulation_width;
const int k_window_height= k_simulation_height;

const int k_view_width= k_window_width/2;
const int k_view_height= k_window_height/2;

// assume 4:3 camera aspect ratio...
const int k_scaled_camera_width= k_view_height*4/3;
const int k_scaled_camera_height= k_view_height;
const int k_scaled_camera_x= (k_view_width-k_scaled_camera_width)/2;
const int k_scaled_camera_y= 0;

// ...and use the center 16:9 portion of it
const int k_scaled_clip_width= k_scaled_camera_width;
const int k_scaled_clip_height= k_scaled_camera_width*9/16;
const int k_scaled_clip_x= (k_scaled_camera_width-k_scaled_clip_width)/2;
const int k_scaled_clip_y= (k_scaled_camera_height-k_scaled_clip_height)/2;

// video frame is top left quarter of the debug view
const SDL_Rect k_video_rect= {k_scaled_camera_x, k_scaled_camera_y, k_scaled_camera_width, k_scaled_camera_height};
const SDL_Rect k_video_clip_rect= {k_video_rect.x+k_scaled_clip_x, k_video_rect.y+k_scaled_clip_y, k_scaled_clip_width, k_scaled_clip_height};

// depth frame is top right quarter of the debug view
const SDL_Rect k_depth_rect= {k_view_width+k_scaled_camera_x, k_scaled_camera_y, k_scaled_camera_width, k_scaled_camera_height};
const SDL_Rect k_depth_clip_rect= {k_depth_rect.x+k_scaled_clip_x, k_depth_rect.y+k_scaled_clip_y, k_scaled_clip_width, k_scaled_clip_height};

// edge frame is the lower left quarter of the debug view
const SDL_Rect k_edge_rect= {0, k_view_height, k_view_width, k_view_height};

// swarm is the lower right quarter of the debug view
const SDL_Rect k_swarm_rect= {k_view_width, k_view_height, k_view_width, k_view_height};

const char *k_bee_texture_filepaths[bee_t::k_state_count]=
{
	"res/32_Idle_Sheet.bmp",
	"res/32_Crawl_Sheet.bmp",
	"res/32_Fly_Sheet.bmp"
};

static SDL_Texture *graphics_create_texture_from_image_file(const char *filePath, int &width, int &height);
static SDL_Texture *graphics_create_texture_from_video_frame(const cv::Mat3b &video_frame);
static SDL_Texture *graphics_create_texture_from_depth_frame(const cv::Mat1w &depth_frame);
static SDL_Texture *graphics_create_texture_from_edge_frame(const cv::Mat1b &edge_frame);
static SDL_Texture *graphics_create_texture_from_string(const char *string, const SDL_Color &color, int &width, int &height);

SDL_Window *g_window= NULL;
SDL_Renderer *g_renderer= NULL;
TTF_Font *g_font= NULL;
SDL_Texture *g_bee_textures[bee_t::k_state_count]= {NULL, NULL, NULL};
int g_bee_sprite_counts[bee_t::k_state_count]= {0, 0, 0};
int g_bee_sprite_sizes[bee_t::k_state_count]= {0, 0, 0};

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
				for (int state= 0; state<bee_t::k_state_count; state++)
				{
					int texture_width, texture_height;

					g_bee_textures[state]= graphics_create_texture_from_image_file(k_bee_texture_filepaths[state], texture_width, texture_height);

					if (g_bee_textures[state])
					{
						// assume square sprites
						g_bee_sprite_counts[state]= texture_width/texture_height;
						g_bee_sprite_sizes[state]= texture_height;
					}
				}

				if (g_bee_textures[bee_t::_idle] && g_bee_textures[bee_t::_crawling] && g_bee_textures[bee_t::_flying])
				{
					g_frame_count= 0;
					g_last_frame_time= SDL_GetPerformanceCounter();
					g_frame_rate= k_fps;

					SDL_SetRenderDrawBlendMode(g_renderer, SDL_BLENDMODE_BLEND);

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
	for (int state= bee_t::k_state_count-1; state>=0; state--)
	{
		if (g_bee_textures[state])
		{
			SDL_DestroyTexture(g_bee_textures[state]);
			g_bee_textures[state]= NULL;
			g_bee_sprite_counts[state]= 0;
			g_bee_sprite_sizes[state]= 0;
		}
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

bool graphics_change_mode(bool fullscreen)
{
	bool success= false;

	if (g_window)
	{
		if (SDL_SetWindowFullscreen(g_window, fullscreen ? SDL_WINDOW_FULLSCREEN : 0)==0)
		{
			success= true;
		}
		else
		{
			SDL_LogError(SDL_LOG_CATEGORY_RENDER, "Failed to change mode to %s: %s", fullscreen ? "fullscreen" : "windowed", SDL_GetError());
		}
	}

	return success;
}

int graphics_render(const swarm_t &swarm, bool debug, const cv::Mat3b &video_frame, const cv::Mat1w &depth_frame, const cv::Mat1b &edge_frame, const commands_t &commands, bool fps)
{
	if (g_renderer)
	{
		// clear background
		if (debug)
		{
			SDL_SetRenderDrawColor(g_renderer, 0x00, 0x00, 0x00, 0xff);
		}
		else
		{
			SDL_SetRenderDrawColor(g_renderer, 0xff, 0xff, 0xff, 0xff);
		}
		SDL_RenderClear(g_renderer);

		// render bees
		{
			float ox= static_cast<float>(debug ? k_swarm_rect.x : 0);
			float oy= static_cast<float>(debug ? k_swarm_rect.y : 0);
			float dx= static_cast<float>(debug ? k_swarm_rect.w : k_window_width)/k_simulation_width;
			float dy= static_cast<float>(debug ? k_swarm_rect.h : k_window_height)/k_simulation_height;

			if (debug)
			{
				const SDL_Color colors[bee_t::k_state_count]=
				{
					{0xff, 0x7f, 0x00, 0xff},
					{0xff, 0xbf, 0x00, 0xff},
					{0xff, 0xff, 0x00, 0xff}
				};

				for (int state= 0; state<bee_t::k_state_count; state++)
				{
					SDL_FPoint points[k_bee_count];
					int point_count= 0;

					for (int bee_index= 0; bee_index<k_bee_count; bee_index++)
					{
						const bee_t *bee= &swarm.bees[bee_index];

						if (bee->state==state)
						{
							SDL_FPoint *point= &points[point_count++];

							point->x= ox + bee->x*dx;
							point->y= oy + bee->y*dy;
						}
					}

					SDL_SetRenderDrawColor(g_renderer, colors[state].r, colors[state].g, colors[state].b, colors[state].a);
					SDL_RenderDrawPointsF(g_renderer, points, point_count);
				}
			}
			else
			{
				int64_t sprite_base_index= static_cast<int64_t>(swarm.t/k_dt);
				SDL_Rect src_rect;
				SDL_FRect dst_rect;

				src_rect.y= 0;

				dst_rect.w= 2*k_bee_radius*dx;
				dst_rect.h= 2*k_bee_radius*dy;

				for (int state= 0; state<bee_t::k_state_count; state++)
				{
					SDL_Texture *texture= g_bee_textures[state];
					int sprite_count= g_bee_sprite_counts[state];
					int sprite_size= g_bee_sprite_sizes[state];

					src_rect.w= sprite_size;
					src_rect.h= sprite_size;

					for (int bee_index= 0; bee_index<k_bee_count; bee_index++)
					{
						const bee_t *bee= &swarm.bees[bee_index];

						if (bee->state==state)
						{
							src_rect.x= ((sprite_base_index+bee_index)%sprite_count)*sprite_size;

							dst_rect.x= ox + (bee->x-k_bee_radius)*dx;
							dst_rect.y= oy + (bee->y-k_bee_radius)*dy;

							SDL_RenderCopyExF(g_renderer, texture,  &src_rect,  &dst_rect, 57.2957795131*bee->facing+90.0, NULL, SDL_FLIP_NONE);
						}
					}
				}
			}
		}

		// render landed
		if (debug)
		{
			float ox= static_cast<float>(debug ? k_swarm_rect.x : 0);
			float oy= static_cast<float>(debug ? k_swarm_rect.y : 0);
			float dx= static_cast<float>(debug ? k_swarm_rect.w : k_window_width)/swarm.landed.cols;
			float dy= static_cast<float>(debug ? k_swarm_rect.h : k_window_height)/swarm.landed.rows;

			SDL_SetRenderDrawColor(g_renderer, 0xbf, 0x55, 0x00, 0x3f);

			for (int y= 0; y<swarm.landed.rows; y++)
			{
				for (int x= 0; x<swarm.landed.cols; x++)
				{
					int landed= swarm.landed(y, x);

					if (landed>0)
					{
						SDL_FRect rect;
						rect.x= ox + x*dx;
						rect.y= oy + y*dy;
						rect.w= dx;
						rect.h= dy;
						SDL_SetRenderDrawColor(g_renderer, 0xbf, 0x55, 0x00, 0xff*landed/swarm.landed_max);
						SDL_RenderFillRectF(g_renderer, &rect);
					}
				}
			}
		}

		// render flow
		if (debug && swarm.flow_active)
		{
			float ox= static_cast<float>(debug ? k_swarm_rect.x : 0);
			float oy= static_cast<float>(debug ? k_swarm_rect.y : 0);
			float dx= static_cast<float>(debug ? k_swarm_rect.w : k_window_width)/swarm.flow.cols;
			float dy= static_cast<float>(debug ? k_swarm_rect.h : k_window_height)/swarm.flow.rows;

			SDL_SetRenderDrawColor(g_renderer, 0x00, 0xff, 0x00, 0xff);

			for (int y= 0; y<swarm.flow.rows; y++)
			{
				for (int x= 0; x<swarm.flow.cols; x++)
				{
					float flow= swarm.flow(y, x);
					float x0= ox + (x+0.5f)*dx;
					float y0= oy + (y+0.5f)*dy;
					float x1= x0 + 0.5f*dx*cos(flow);
					float y1= y0 + 0.5f*dy*sin(flow);

					SDL_RenderDrawLineF(g_renderer, x0, y0, x1, y1);
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
			}

			SDL_SetRenderDrawColor(g_renderer, 0x00, 0x00, 0xff, 0xff);
			SDL_RenderDrawRect(g_renderer, &k_video_clip_rect);
		}

		// render depth
		if (debug)
		{
			SDL_Texture *depth_texture= graphics_create_texture_from_depth_frame(depth_frame);

			if (depth_texture)
			{
				SDL_RenderCopy(g_renderer, depth_texture, NULL, &k_depth_rect);
				SDL_DestroyTexture(depth_texture);
			}

			SDL_SetRenderDrawColor(g_renderer, 0x00, 0x00, 0xff, 0xff);
			SDL_RenderDrawRect(g_renderer, &k_depth_clip_rect);
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

		// render commands
		if (debug)
		{
			for (int command_index= 0; command_index<commands.size(); command_index++)
			{
				const command_t *command= &commands[command_index];
				SDL_Rect command_rect;

				command_rect.x= k_video_clip_rect.x + command->bounding_box.x*k_video_clip_rect.w/edge_frame.cols;
				command_rect.y= k_video_clip_rect.y + command->bounding_box.y*k_video_clip_rect.h/edge_frame.rows;
				command_rect.w= command->bounding_box.width*k_video_clip_rect.w/edge_frame.cols;
				command_rect.h= command->bounding_box.height*k_video_clip_rect.h/edge_frame.rows;

				SDL_SetRenderDrawColor(g_renderer, 0x00, 0x00, 0xff, 0xff);
				SDL_RenderDrawRect(g_renderer, &command_rect);
			}
		}

		// render frame rate
		if (fps)
		{
			char frame_rate_string[4];
			snprintf(frame_rate_string, sizeof(frame_rate_string), "%3.0f", g_frame_rate);
			SDL_Color color= {0x0, 0xff, 0x0, 0xff};
			int width, height;
			SDL_Texture *fps_texture= graphics_create_texture_from_string(frame_rate_string, color, width, height);

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

static SDL_Texture* graphics_create_texture_from_image_file(const char *filepath, int &width, int &height)
{
	SDL_Texture *texture= NULL;
	SDL_Surface *surface= SDL_LoadBMP(filepath);

	width= 0;
	height= 0;

	if (surface)
	{
		int surface_width= surface->w;
		int surface_height= surface->h;

		texture= SDL_CreateTextureFromSurface(g_renderer, surface);
		SDL_FreeSurface(surface);

		if (texture)
		{
			width= surface_width;
			height= surface_height;

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

			for (int y= 0; y<surface->h; y++)
			{
				uint8_t *color= color_row;

				for (int x= 0; x<surface->w; x++)
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

			for (int y= 0; y<surface->h; y++)
			{
				uint8_t *color= color_row;

				for (int x= 0; x<surface->w; x++)
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

static SDL_Texture *graphics_create_texture_from_string(const char *string, const SDL_Color &color, int &width, int &height)
{
	SDL_Texture *texture= NULL;

	width= 0;
	height= 0;

	if (g_font)
	{
		SDL_Surface *surface= TTF_RenderText_Blended(g_font, string, color);

		if (surface)
		{
			int surface_width= surface->w;
			int surface_height= surface->h;

			texture= SDL_CreateTextureFromSurface(g_renderer, surface);
			SDL_FreeSurface(surface);

			if (texture)
			{
				width= surface_width;
				height= surface_height;

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
