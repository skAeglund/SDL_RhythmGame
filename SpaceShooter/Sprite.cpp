#include "Sprite.h"

#include <algorithm>

#include "engine.h"
#include <iostream>

using namespace std;

//Sprite::Sprite() {};
void Sprite::load(const char* path, SDL_Renderer* render)
{
	this->path = path;
	texture = IMG_LoadTexture(render, path);
	renderer = render;

	const int result = SDL_QueryTexture(texture, NULL, NULL, &spriteWidth, &spriteHeight);
	if (result != 0)
	{
		cout << "Failed to load image at: " << path << endl;
	}
}
void Sprite::draw(int x, int y, int width, int height, SDL_Renderer* render)
{
	const SDL_Rect destination{ x - (scaleOffset /2), y - (scaleOffset / 2), width + scaleOffset, height + scaleOffset};
	const int result = SDL_RenderCopy(render == nullptr ? renderer : render, texture, NULL, &destination);
	if (result != 0)
	{
		cout << "Fail with: " << path << endl;
	}
}
void Sprite::drawRotated(int x, int y, int width, int height, float angle, SDL_Renderer* render)
{
	const SDL_Rect destination{ x - (scaleOffset / 2), y - (scaleOffset / 2), width + scaleOffset, height + scaleOffset };
	SDL_RenderCopyEx(render, texture, NULL, &destination, angle, NULL, SDL_RendererFlip::SDL_FLIP_NONE);
}

void Sprite::updateOpacity(float progress)
{
	//SDL_clamp(progress, 0.1f, 1);
	progress = clamp(progress, 0.f, 1.f);
	const Uint8 a = static_cast<Uint8>(255 * progress);
	SDL_SetTextureAlphaMod(texture, a);
}

void Sprite::destroy()
{
	SDL_DestroyTexture(texture);
}
//todo: destroy()
