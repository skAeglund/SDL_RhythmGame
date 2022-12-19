#pragma once
#include <SDL.h>
#include <iostream>
#include <stdlib.h>
#include <vector>
#include "collision.h"
#include "player.h"
#include "gameObject.h"
#include "musicManager.h"

#define WIDTH 1920
#define HEIGHT 1080
using namespace GameObject;

/// <summary>
/// This manages all objects of the game including movment,
/// collision/depenetration and rendering
/// A future improvement would be to move out game-specific stuff
/// like "laser" to a more fitting location
/// </summary>
namespace Engine
{
	bool initializeEngine(const char* textures[], int size);
	void quit();

	// ----------- Rendering----------------
	SDL_Renderer* getRenderer();
	void drawObjects();
	void drawCircles(float x, float y, float radius);
	void drawBeatCircles(MusicData* musicData);
	void drawLasers(SDL_Color color);
	void drawHealthLine(MusicData* musicData);
	void drawColliders();
	void drawBackground();
	void renderPresent();
	void renderClear();
	void unloadTextures();

	// ---------- Object handling------------
	void createObject(Position pos, Rotation rot, Velocity vel, float scaleOffset = 0, SDL_Texture* texture = nullptr, Tag tag = Tag::Asteroid);
	void createObject(Position position, Rotation rotation, Velocity velocity, float scaleOffset, const char* texturePath, Tag tag = Tag::Asteroid);
	void removeObject(int index);
	void clearObjects();
	bool addLaser(Laser line, MusicData* musicData);
	void sortObjects();
	void moveObjects();
	void rotateObjects();
	int checkForObjectDestruction(MusicManager& musicManager);
	void updatePlayerVelocity(float x, float y);
	int getObjectCount();
	Position getPlayerPos();
	
	// ----------- Time related ------------ 
	float updateTicks();
	void delayNextFrame();
	void printTimeStats();
	
	// ----------- Key related ------------- 
	bool getKeyDown(int index);
	void updateKey(int index, bool value);
	void resetKeys();
}


