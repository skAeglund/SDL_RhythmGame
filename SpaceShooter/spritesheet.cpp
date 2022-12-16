#include "spritesheet.h"
#include "engine.h"
#include <sstream>
#include <math.h>
#include <iostream>
#define FPS 25
using namespace std;

// gets the source (part of the sheet to render) and destination (where to render)
void SpriteSheet::getTextureRects(int x, int y, float elapsedTime, SDL_Rect* source, SDL_Rect* destination)
{
	int index = (int)(elapsedTime * FPS) % (rows * columns);
	int xPos = (index % (rows)) * width;
	int yPos = (index / rows) * height;
	*source = { xPos, yPos, width, height };
	*destination = { x,y, width, height };
}
void SpriteSheet::load(const char* path, int spriteWidth, int spriteHeight, int rows, int columns, SDL_Renderer* render)
{
	texture = IMG_LoadTexture(render, path);
	int result = SDL_QueryTexture(texture, NULL, NULL, &width, &height);
	if (result != 0)
	{
		cout << "Failed to load image at: " << path << endl;
	}
	width = spriteWidth, height = spriteHeight;
	this->rows = rows, this->columns = columns;
	SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
}
// draws a frame based on time (25fps)
void SpriteSheet::draw( int x, int y, float elapsedTime, SDL_Renderer* render)
{
	SDL_Rect source, destination;
	getTextureRects(x, y, elapsedTime, &source, &destination);
	SDL_RenderCopy(render, texture, &source, &destination);
}
// draws a specific frame
void SpriteSheet::draw(int frame, int x, int y, SDL_Renderer* render)
{
	int index = frame % (rows * columns);
	int xPos = (index % (rows)) * width;
	int yPos = (index / rows) * height;
	SDL_Rect source = { xPos, yPos, width, height };
	SDL_Rect destination = { x,y, width, height };
	SDL_RenderCopy(render, texture, &source, &destination);
}
void SpriteSheet::drawRotated(int x, int y, float angle, float elapsedTime, SDL_Renderer* render)
{
	SDL_Rect source, destination;
	getTextureRects(x, y, elapsedTime ,&source, &destination);
	SDL_RenderCopyEx(render, texture, &source, &destination, angle, NULL, SDL_RendererFlip::SDL_FLIP_NONE);
}
void SpriteSheet::drawAtAngle(int x, int y, double angle, SDL_Point& point, float t, float elapsedTime, SDL_Renderer* render)
{
	// NOTE: this is used only for rendering the trail behind the ball
	// todo: move or make modular / adaptable
	SDL_Rect source, destination;
	getTextureRects(x, y, elapsedTime, &source, &destination);
	destination.w = lerp(10, width, t);

	Uint8 alpha = (int)lerp(100, 255, t);
	SDL_SetTextureAlphaMod(texture, alpha);
	
	SDL_RenderCopyEx(render, texture, &source, &destination, angle, &point, SDL_RendererFlip::SDL_FLIP_NONE);
}

//template <size_t S>
void MultiSpriteSheet::load(const char* basePath, const char* type, int spriteWidth, int spriteHeight, int rows, int columns, SDL_Renderer* render)
{
	if (sheetCount == 0) sheetCount = (int)size(textures);

	for (int i = 0; i < sheetCount; i++)
	{
		string path = basePath + to_string(i) + type/*".jpg"*/;
		textures[i] = IMG_LoadTexture(render, path.c_str());
		int result = SDL_QueryTexture(textures[i], NULL, NULL, &width, &height);
		if (result != 0)
		{
			cout << "Failed to load image at: " << path << endl;
		}
	}
	width = spriteWidth, height = spriteHeight;
	this->rows = rows, this->columns = columns;
}
//template <size_t S>
void MultiSpriteSheet::draw(int x, int y, float elapsedTime, SDL_Renderer* render)
{
	int spritesPerSheet = rows * columns;
	int index = (int)(elapsedTime * FPS) % ((rows * columns) * sheetCount);
	int sheetIndex = index / spritesPerSheet;
	index = index % spritesPerSheet;

	int xPos = (index % (rows)) * width;
	int yPos = (index / columns) * height;

	if (sheetCount ==5) y = sheetIndex == 2 ? 1 : 0; // temporary fix for failed spritesheet

	SDL_Rect source{ xPos, yPos, width, height };
	SDL_Rect destination{ x,y, width, height };
	SDL_RenderCopy(render, textures[sheetIndex], &source, &destination);
}
void MultiSpriteSheet::setCount(int count)
{
	sheetCount = count;
}