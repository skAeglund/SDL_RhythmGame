#pragma once
#include <map>
#include <SDL.h>
#define MAX_BUTTON_COUNT 2

namespace UI
{
	enum class Menu { intro, pause, gameOver };
	enum class Button { start, restart, resume, quit, none };

	struct TextureSize { int width; int height; };
	struct Texture { SDL_Texture* texture; TextureSize size; };

	// Note: this structure is based on (my understanding of) data-oriented programming,
	// in spirit of the assignment.
	// A possible improvement would be to create a Menu class with buttons, overlays etc. 
	struct Buttons
	{
		Button types[MAX_BUTTON_COUNT];
		Texture textures[MAX_BUTTON_COUNT];
		SDL_Rect rects[MAX_BUTTON_COUNT];
		size_t count;

		Buttons(SDL_Renderer* render, Menu menu);
		void draw(SDL_Renderer* render, int mouseX, int mouseY, Button clickedButton = Button::none, float opacityMultiplier = -1.f);
		Button isIntersectingAnyButton(int mouseX, int mouseY);

		void unloadTextures();

		// Atm, each menu only have one specific button (+ quit) but I made this for the sake of modularity 
		const std::map<Menu, std::initializer_list<Button>> menuButtonMap
		{
			{ Menu::intro, { Button::start, Button::quit }},
			{ Menu::pause, { Button::resume, Button::quit } },
			{ Menu::gameOver, { Button::restart, Button::quit }}
		};
	};
}
