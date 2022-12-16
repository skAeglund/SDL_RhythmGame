#include "crosshair.h"
#include "SDL_image.h"

Crosshair::Crosshair(int width, int height, const char* path, SDL_Renderer* render)
{
	rect.w = width;
	rect.h = height;
	texture = IMG_LoadTexture(render, path);
	SDL_ShowCursor(false);
}

void Crosshair::update()
{
	SDL_GetMouseState(&rect.x, &rect.y);
	rect.x -= rect.w / 2;
	rect.y -= rect.h / 2;
}

void Crosshair::draw(SDL_Renderer* render)
{
	SDL_RenderCopy(render, texture, NULL, &rect);
}

void Crosshair::destroy()
{
	SDL_DestroyTexture(texture);
}
