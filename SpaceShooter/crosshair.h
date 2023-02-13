#pragma once
#include <SDL.h>
#include "gameObject.h"

struct Crosshair
{
	SDL_Texture* texture;
	SDL_Rect rect;
	const GameObject::Color normalColor{ 0, 225, 255, 255 };
	const GameObject::Color failColor{ 255, 0, 0, 255 };

	Crosshair(int width, int height, const char* path, SDL_Renderer* render);

	void draw(SDL_Renderer* render, float quarterProgress, float timeSinceLastFail);
	void destroy();
};
