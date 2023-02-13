#pragma once
#include <SDL_image.h>

/// <summary>
/// This is mainly used for single immovable things like screen overlays
/// that last for a limited amount of time
/// Texture rendering for all movable objects is done in engine.cpp
/// </summary>
struct Sprite
{
	int width, height;
	int scaleOffset = 0;
	SDL_Texture* texture;
	const char* path;
	SDL_Renderer* renderer;

	int spriteWidth, spriteHeight;

	void load(const char* path, SDL_Renderer* render);
	void draw(int x, int y, int width, int height, SDL_Renderer* render = nullptr);
	void drawRotated(int x, int y, int width, int height, float angle, SDL_Renderer* render);
	void updateOpacity(float progress);
	void destroy();
};