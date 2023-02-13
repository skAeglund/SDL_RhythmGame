#include "UI.h"
#include <iostream>
#include <map>
#include <SDL_image.h>

#include "assets.h"
#include "engine.h"
constexpr auto HOVER_MULTIPLIER = 1.04f;

namespace UI
{
	// fullscreen 
#if HEIGHT == 1080
	const std::map<Button, SDL_Rect> buttonRectMap
	{
		{ Button::start, SDL_Rect{ 503,572,405,115 } },
		{ Button::restart, SDL_Rect{503,572,405,115 } },
		{ Button::resume, SDL_Rect{503,572,405,115 } },
		{ Button::quit, SDL_Rect{1013,572,405,115} }
	};
	// 720p 
#else
	const std::map<Button, SDL_Rect> buttonRectMap
	{
		{ Button::start, SDL_Rect{ 335,381,270,77 } },
		{ Button::restart, SDL_Rect{335,381,270,77 } },
		{ Button::resume, SDL_Rect{335,381,270,77 } },
		{ Button::quit, SDL_Rect{675,381,270,77} }
	};
#endif

	// initializes the buttons needed for the specific menu
	Buttons::Buttons(SDL_Renderer* render, Menu menu) : types{}, textures{}, rects{}
	{
		const std::initializer_list<Button> buttons = menuButtonMap.at(menu);
		count = buttons.size();
		size_t i = 0;
		for (auto button : buttons)
		{
			types[i] = button;
			rects[i] = buttonRectMap.at(button);
			SDL_Texture* texture = IMG_LoadTexture(render, Assets::buttonPathMap.at(button));
			TextureSize size{};
			const int result = SDL_QueryTexture(texture, NULL, NULL, &size.width, &size.height);
			if (result != 0)
			{
				std::cout << "Failed to load image at: " << Assets::buttonPathMap.at(button) << std::endl;
				return; // todo: throw exception
			}

			textures[i] = Texture(texture, size);
			i++;
		}
	}

	void Buttons::draw(SDL_Renderer* render, int mouseX, int mouseY, Button clickedButton, float opacityMultiplier)
	{
		const SDL_Point point{ mouseX, mouseY };
		for (size_t i = 0; i < count; i++)
		{
			SDL_Rect destination = { rects[i].x, rects[i].y, rects[i].w, rects[i].h };
			SDL_Rect source = { 0, 0, textures[i].size.width / 3, textures[i].size.height };

			if (types[i] == clickedButton)
			{
				source.x += (textures[i].size.width / 3) * 2;
				opacityMultiplier = 1 - powf(1 - opacityMultiplier, 2);
			}
			else if (SDL_PointInRect(&point, &rects[i]))
			{
				source.x += textures[i].size.width / 3;
				destination = {
					rects[i].x - static_cast<int>(rects[i].w * (HOVER_MULTIPLIER - 1)) / 2,
					rects[i].y - static_cast<int>(rects[i].h * (HOVER_MULTIPLIER - 1)) / 2,
					static_cast<int>(rects[i].w * HOVER_MULTIPLIER),
					static_cast<int>(rects[i].h * HOVER_MULTIPLIER)
				};
			}
			else
			{
				opacityMultiplier = powf(opacityMultiplier, 2);
			}

			if (opacityMultiplier != -1)
			{
				SDL_SetTextureAlphaMod(textures[i].texture, static_cast<Uint8>(255 * opacityMultiplier));
			}
			const int result = SDL_RenderCopy(render, textures[i].texture, &source, &destination); // source will be used to select which part of the texture to use

			if (result != 0)
			{
				std::cout << "Error: Failed to render Button." << std::endl;
			}
		}
	}

	Button Buttons::isIntersectingAnyButton(int mouseX, int mouseY)
	{
		const SDL_Point point{ mouseX, mouseY };
		for (int i = 0; i < count; i++)
		{
			if (SDL_PointInRect(&point, &rects[i]))
			{
				return types[i];
			}
		}
		return Button::none;
	}
	void Buttons::unloadTextures()
	{
		for (int i = 0; i < count; i++)
		{
			SDL_DestroyTexture(textures[i].texture);
		}
	}
}
