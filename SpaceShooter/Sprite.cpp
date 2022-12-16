#include "Sprite.h"
#include "engine.h"
#include <iostream>

using namespace std;
int spriteWidth, spriteHeight;

//Sprite::Sprite() {};
void Sprite::load(const char* path, SDL_Renderer* render, bool randomizeTint)
{
	this->path = path;
	texture = IMG_LoadTexture(render, path);
	if (randomizeTint)
	{
		float r = rand() % 25 + 200, g = rand() % 20 + 220, b = rand() % 15 + 240; // randomized color tint for each instance
		SDL_SetTextureColorMod(texture, r, g, b);
	}
	int result = SDL_QueryTexture(texture, NULL, NULL, &spriteWidth, &spriteHeight);
	if (result != 0)
	{
		cout << "Failed to load image at: " << path << endl;
	}
}
void Sprite::draw(int x, int y, int width, int height, SDL_Renderer* render)
{
	SDL_Rect destination{ x - (scaleOffset /2), y - (scaleOffset / 2), width + scaleOffset, height + scaleOffset};
	int result = SDL_RenderCopy(render, texture, NULL, &destination);
	if (result != 0)
	{
		cout << "Fail with: " << path << endl;
	}
}
void Sprite::drawRotated(int x, int y, int width, int height, float angle, SDL_Renderer* render)
{
	SDL_Rect destination{ x - (scaleOffset / 2), y - (scaleOffset / 2), width + scaleOffset, height + scaleOffset };
	SDL_RenderCopyEx(render, texture, NULL, &destination, angle, NULL, SDL_RendererFlip::SDL_FLIP_NONE);
}
void Sprite::destroy()
{
	SDL_DestroyTexture(texture);
}
//todo: destroy()
