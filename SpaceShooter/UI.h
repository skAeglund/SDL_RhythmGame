#pragma once
#include <SDL.h>
#define TOTAL_BUTTON_COUNT 4


enum class Menu { intro, pause, gameOver };
enum class ButtonType { start, restart, resume, quit, none };
struct TextureSize { int width; int height; };
struct Texture { SDL_Texture* texture; TextureSize size;};

//template<int Size>
struct Buttons
{
	ButtonType types[TOTAL_BUTTON_COUNT];
	Texture textures[TOTAL_BUTTON_COUNT];
	SDL_Rect rects[TOTAL_BUTTON_COUNT];
	int count;

	Buttons(SDL_Renderer* render, ButtonType buttonTypes[], int count);
	void draw(SDL_Renderer* render, int mouseX, int mouseY, ButtonType clickedButton = ButtonType::none, float opacityMultiplier = -1.f);
	ButtonType isIntersectingAnyButton(int mouseX, int mouseY);

	void unloadTextures();
};