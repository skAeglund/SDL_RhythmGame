#include "UI.h"
#include <iostream>
#include <map>
#include <SDL_image.h>

#include "assets.h"

constexpr auto HOVER_MULTIPLIER = 1.04f;

const std::map<ButtonType, SDL_Rect> buttonRectMap
{
	{ ButtonType::start, SDL_Rect{ 503,572,405,115 } },
	{ ButtonType::restart, SDL_Rect{503,572,405,115 } },
	{ ButtonType::resume, SDL_Rect{503,572,405,115 } }, 
	{ ButtonType::quit, SDL_Rect{1013,572,405,115} }
};

//template<int Size>
Buttons::Buttons(SDL_Renderer* render, ButtonType buttonTypes[], int count) : count(count)
{

	for (int i = 0; i < count; i++)
	{
		types[i] = buttonTypes[i];
		rects[i] = buttonRectMap.at(types[i]);
		SDL_Texture* texture = IMG_LoadTexture(render, Assets::buttonPathMap.at(types[i]));
		TextureSize textureSize;
		int result = SDL_QueryTexture(texture, NULL, NULL, &textureSize.width, &textureSize.height);
		if (result != 0)
		{
			std::cout << "Failed to load image at: " << Assets::buttonPathMap.at(types[i]) << std::endl;
			return; // todo: throw exception
		}
	
		textures[i] = Texture(texture, textureSize);
		//rects[i].w = textureSize.width / 3;
		//rects[i].h = textureSize.height;
	}
}

//template <int Size>
void Buttons::draw(SDL_Renderer* render, int mouseX, int mouseY, ButtonType clickedButton, float opacityMultiplier)
{
	const SDL_Point point{ mouseX, mouseY };
	for (int i = 0; i < count; i++)
	{
		SDL_Rect destination = { rects[i].x, rects[i].y, rects[i].w, rects[i].h };
		SDL_Rect source = { 0, 0, textures[i].size.width / 3, textures[i].size.height };

		if (types[i] == clickedButton)
		{
			source.x += (textures[i].size.width / 3) * 2;
			opacityMultiplier = 1 - pow(1 - opacityMultiplier, 2);
		}
		else if (SDL_PointInRect(&point, &rects[i]))
		{
			source.x += textures[i].size.width / 3;
			destination = {
				rects[i].x - (int)(rects[i].w * (HOVER_MULTIPLIER - 1)) / 2,
				rects[i].y - (int)(rects[i].h * (HOVER_MULTIPLIER - 1)) / 2,
				(int)(rects[i].w * HOVER_MULTIPLIER),
				(int)(rects[i].h * HOVER_MULTIPLIER)
			};
		}
		else
		{
			opacityMultiplier = pow(opacityMultiplier, 2);
		}
		
		if (opacityMultiplier != -1)
		{
			SDL_SetTextureAlphaMod(textures[i].texture, static_cast<Uint8>(255 * opacityMultiplier));
		}
		const int result = SDL_RenderCopy(render, textures[i].texture, &source, &destination); // source will be used to select with part of the texture to use

		if (result != 0)
		{
			std::cout << "Error: Failed to render ButtonType." << std::endl;
		}
	}
}

//template <int Size>
ButtonType Buttons::isIntersectingAnyButton(int mouseX, int mouseY)
{
	const SDL_Point point{ mouseX, mouseY };
	for (int i = 0; i < count; i++)
	{
		if (SDL_PointInRect(&point, &rects[i]))
		{
			return types[i];
		}
	}
	return ButtonType::none;
}
//template <int Size>
void Buttons::unloadTextures()
{
	for (int i = 0; i < count; i++)
	{
		SDL_DestroyTexture(textures[i].texture);
	}
}


// to avoid link errors, could also move the implementation to the header file... or include this file (which feels wrong)
//template class Buttons<1>; template class Buttons<2>;
//template class Buttons<3>; template class Buttons<4>;