#include "crosshair.h"

#include <algorithm>

#include "easingFunctions.h"
#include "SDL_image.h"
#include "rendering.h"

Crosshair::Crosshair(int width, int height, const char* path, SDL_Renderer* render)
{
	rect.w = width;
	rect.h = height;
	texture = IMG_LoadTexture(render, path);
	SDL_ShowCursor(false);
}

void Crosshair::draw(SDL_Renderer* render, float quarterProgress, float timeSinceLastFail)
{
	SDL_GetMouseState(&rect.x, &rect.y);
	int mouseX = rect.x;
	int mouseY = rect.y;
	rect.x -= rect.w / 2;
	rect.y -= rect.h / 2;

	// render texture
	SDL_RenderCopy(render, texture, NULL, &rect);

	Color color = normalColor;
	if (timeSinceLastFail > 0 && timeSinceLastFail < 2)
	{
		color = Color{
			std::lerp(failColor.r, normalColor.r, timeSinceLastFail / 2.f),
			std::lerp(failColor.g, normalColor.g, timeSinceLastFail / 2.f),
			std::lerp(failColor.b, normalColor.b, timeSinceLastFail / 2.f),
			255.f
		};

		const UINT8 textureColorValue = static_cast<UINT8>(255 * std::clamp(timeSinceLastFail / 1.f, 0.f, 1.f));
		SDL_SetTextureColorMod(texture, 255, textureColorValue, textureColorValue);
	}

	const int layerCount = static_cast<int>(std::lerp(12.f, 6.f, Ease::Out(quarterProgress, 3)));
	// draw beat blinking layers
	for (size_t j = 0; j < layerCount; j++)
	{
		float multiplier = 1 - (static_cast<float>(j) / static_cast<float>(layerCount));
		multiplier = powf(multiplier, 2);

		const auto a = static_cast<UINT8>(255 * multiplier * (1 - (Ease::Out(quarterProgress, 5) * 0.9f)));
		color = color.multiplied(multiplier);
		SDL_SetRenderDrawColor(render, color.r, color.g, color.b, a);
		Rendering::drawHexagon(mouseX + 0.4f, mouseY + 0.4f, (8 + j + 7.f), 0.5f);
		Rendering::drawHexagon(mouseX + 0.4f, mouseY + 0.4f, (8 - j * 0.5f));
	}
}

void Crosshair::destroy()
{
	SDL_DestroyTexture(texture);
}
