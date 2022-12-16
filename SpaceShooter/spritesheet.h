#pragma once
#include "SDL_image.h"
#include <vector>

struct SpriteSheet
{
	//int x, y;
	int width, height;
	int rows, columns;
	SDL_Texture* texture;

	void load(const char* path, int spriteWidth, int spriteHeight, int rows, int columns, SDL_Renderer* render);
	void draw(int x, int y, float elapsedTime, SDL_Renderer* render);
	void draw(int frame, int x, int y, SDL_Renderer* render);
	void drawRotated(int x, int y, float angle, float elapsedTime, SDL_Renderer* render);
	void drawAtAngle(int x, int y, double angle, SDL_Point& point, float t, float elapsedTime, SDL_Renderer* render);
	void getTextureRects(int x, int y, float elapsedTime, SDL_Rect* source, SDL_Rect* destination);
};

//template <size_t sheetCount>
struct MultiSpriteSheet
{
	int sheetCount;
	int width, height;
	int rows, columns;
	SDL_Texture* textures[5];

	void load(const char* path, const char* type, int spriteWidth, int spriteHeight, int rows, int columns, SDL_Renderer* render);
	void draw(int x, int y, float elapsedTime, SDL_Renderer* render);
	void setCount(int count);
};