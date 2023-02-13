#pragma once
#include <SDL.h>
#include "gameObject.h"
#include "musicManager.h"
#define WIDTH 1920
#define HEIGHT 1080

using namespace GameObject;
struct Player;

/// <summary>
/// This namespace manages all objects of the game
/// including movement and collision/de-penetration.
/// A future improvement would be to make it less game-specific
///	and more modular.
/// </summary>
namespace Engine
{
	bool initializeEngine(const char* textureArr[], size_t size);
	void quit();

	// ---------- Object handling------------
	void createObject(Position pos, Rotation rot, Velocity vel, float scaleOffset = 0, SDL_Texture* texture = nullptr, Tag tag = Tag::Asteroid);
	void createObject(Position position, Rotation rotation, Velocity velocity, float scaleOffset, const char* texturePath, Tag tag = Tag::Asteroid);
	void removeObject(size_t index);
	void clearObjects();
	bool addLaser(Laser line, MusicData* musicData);
	void updateObjects(float wholeNoteLength);
	void sortObjects();
	void moveObjects();
	void rotateObjects();
	void checkForObjectDestruction(Player* player);
	void updatePlayerVelocity(float x, float y);
	int getObjectCount();
	Position getPlayerPos();
	void updatePlayerTextureMod(Color color);
	void createStar(float x, float y, float maxSize, Color color, int lifeTime, float elapsedTime = 0.f);
	void createStartingStars(int count);
	void updateObjectsLifetime(float wholeNoteLength);
	

	// ----------- Rendering----------------
	void drawEverything(MusicData* music, int playerHealth, bool gamePaused = false);
	void unloadTextures();
	
	// ----------- Time related ------------ 
	float updateTicks();
	void delayNextFrame();
	void printTimeStats();
	
	// ----------- Key related ------------- 
	bool getKeyDown(int index);
	void updateKey(int index, bool value);
	void resetKeys();
}
