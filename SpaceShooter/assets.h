#pragma once
#include <map>
#include "musicManager.h"
#include "UI.h"


namespace Assets
{
	inline const char* texturePaths[] = {
		"Content/Sprites/background.png",
		"Content/Sprites/meteor1_blue.png",
		"Content/Sprites/meteor2.png",
		"Content/Sprites/meteor3.png",
		"Content/Sprites/meteor4.png"
	};
	inline Beat beats[NUMBER_OF_BEATS] = {
		Beat(114, 4,  "Content/Audio/Sunset_Loop_16.wav"),
		Beat(120, 4, "Content/Audio/Jupiter_Loop.wav"),
		Beat(133, 4, "Content/Audio/RunningInTheNight_Loop.wav")
	};
	const std::map<ButtonType, const char*> buttonPathMap
	{
		{ ButtonType::start, "Content/Sprites/PlayButton_Sheet.png" },
		{ ButtonType::restart, "Content/Sprites/RestartButton_Sheet.png" },
		{ ButtonType::resume, "Content/Sprites/ResumeButton_Sheet.png" },
		{ ButtonType::quit, "Content/Sprites/QuitButton_Sheet.png" }
	};
	const std::map<Menu, const char*> menuOverlayPaths
	{
		{ Menu::intro, "Content/Sprites/PauseScreen.png"},
		{ Menu::pause, "Content/Sprites/PauseScreen.png"}, // todo: make new overlays
		{ Menu::gameOver, "Content/Sprites/GameOverScreen.png" } // todo: make new overlays
	};
}
