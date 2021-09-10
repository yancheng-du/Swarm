#include <SDL.h>
#include <libfreenect.h>

int main(int argc, char *argv[])
{
	int result= 3;

	if (SDL_Init(SDL_INIT_VIDEO)==0)
	{
		SDL_Window *window;
		SDL_Renderer *renderer;

		if (SDL_CreateWindowAndRenderer(320, 240, SDL_WINDOW_RESIZABLE, &window, &renderer)==0)
		{
			SDL_Surface *surface= SDL_LoadBMP("res/carnage.bmp");

			if (surface)
			{
				SDL_Texture *texture= SDL_CreateTextureFromSurface(renderer, surface);

				SDL_FreeSurface(surface);

				if (texture)
				{
					freenect_context *context;

					if (freenect_init(&context, NULL)==0)
					{
						if (freenect_num_devices(context)>0)
						{
							freenect_device *device;

							if (freenect_open_device(context, &device, 0)==0)
							{
								result= 0;

								while (true)
								{
									SDL_Event event;

									SDL_PollEvent(&event);

									if (event.type==SDL_QUIT)
									{
										break;
									}
									else
									{
										SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
										SDL_RenderClear(renderer);
										SDL_RenderCopy(renderer, texture, NULL, NULL);
										SDL_RenderPresent(renderer);
									}
								}

								freenect_close_device(device);
							}
							else
							{
								SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't open kinect device");
							}
						}
						else
						{
							SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't find any kinect devices");
						}

						freenect_shutdown(context);
					}
					else
					{
						SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create kinect context");
					}

					SDL_DestroyTexture(texture);
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

			SDL_DestroyRenderer(renderer);
			SDL_DestroyWindow(window);
		}
		else
		{
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create window and renderer: %s", SDL_GetError());
		}

		SDL_Quit();
	}
	else
	{
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize SDL: %s", SDL_GetError());
	}

	return result;
}
