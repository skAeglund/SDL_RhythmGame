#pragma once
#include "gameObject.h"
#include "musicManager.h"

using namespace GameObject;

namespace Rendering
{
	SDL_Renderer* getRenderer();
	void setRenderer(SDL_Renderer* renderer);
	void drawObjects(std::vector<Position> positions, std::vector<Appearance> appearances, std::vector<Rotation> rotations);
	void drawObjectsFadingOut(std::vector<ObjectPendingDeletion> objects);
	void drawCircles(float x, float y, float radius);
	void drawBeatCircles(MusicData* music, std::vector<Position> positions, std::vector<Tag> tags, size_t playerIndex);
	void drawLasers(std::vector<Laser> lasers, std::vector<Position> positions, Position playerPos);
	void drawHealthLine(MusicData* musicData, int playerHealth);
	void drawStars(MusicData* music, std::vector<Star> starList, float elapsedTime);
	void drawHexagon(float x, float y, float radius, float offset);
	void drawHexagon(float x, float y, float radius);
	void drawBackground();
	void renderPresent();
	void renderClear();
}