#pragma once
#include "map"
#include "musicManager.h"
#include "UI.h"

/**
 * \brief All paths to assets are gathered here. Textures, SFXs, WAVs etc. 
 */
namespace Assets
{
	inline const char* playerTexturePath = "Content/Sprites/player90x90.png";
	inline const char* crosshairTexturePath = "Content/Sprites/crosshair.png";

	inline const char* laserSFX = "Content/Audio/SFX_Laser";
	inline const char* badLaserSFX = "Content/Audio/SFX_BadLaser.wav";
	inline const char* transitionSFX = "Content/Audio/SFX_Transition.wav";
	inline const char* glitchSFX = "Content/Audio/SFX_CollisionGlitch2.wav";

	inline const char* sphericalLightPath = "Content/Sprites/spherical_light.png";

	inline const char* texturePaths[] = {
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
	const std::map<UI::Button, const char*> buttonPathMap
	{
		{ UI::Button::start, "Content/Sprites/PlayButton_Sheet.png" },
		{ UI::Button::restart, "Content/Sprites/RestartButton_Sheet.png" },
		{ UI::Button::resume, "Content/Sprites/ResumeButton_Sheet.png" },
		{ UI::Button::quit, "Content/Sprites/QuitButton_Sheet.png" }
	};
	const std::map<UI::Menu, const char*> menuOverlayPaths
	{
		{ UI::Menu::intro, "Content/Sprites/PauseScreen.png"}, // todo: make new overlays
		{ UI::Menu::pause, "Content/Sprites/PauseScreen.png"}, 
		{ UI::Menu::gameOver, "Content/Sprites/GameOverScreen.png" } 
	};
}
