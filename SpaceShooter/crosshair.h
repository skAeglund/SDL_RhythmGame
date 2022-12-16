#pragma once
#include <SDL.h>

struct Crosshair
{
	SDL_Texture* texture;
	SDL_Rect rect;

	Crosshair(int width, int height, const char* path, SDL_Renderer* render);
	void update();
	void draw(SDL_Renderer* render);
	void destroy();
};