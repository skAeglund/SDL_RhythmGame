#include "engine.h"
#include "collision.h"
#include "vector2.h"
#include "musicManager.h"
#include "game.h"
#include <algorithm>
#include <Windows.h>
#include <iomanip>
#include <cmath>
#include <iostream>
#include <SDL_image.h>

#include "rendering.h"

using namespace Vector2D;
using namespace Collision;

namespace Engine
{
	SDL_Window* window;
	SDL_Rect windowRect;

	Uint64 previousTicks;
	float deltaTime = 0;
	float elapsedTime = 0;
	int framerate;

	std::vector<Position> positions;
	std::vector<Velocity> velocities;
	std::vector<Rotation> rotations;
	std::vector<Appearance> appearances;
	std::vector<Tag> tags;
	size_t objectCount;
	size_t playerIndex;

	std::vector<Laser> lineList;
	std::vector<ObjectPendingDeletion> objectsToDelete;

	std::vector<Star> starList;

	std::vector<SDL_Texture*> textures;

	bool keys[SDL_NUM_SCANCODES] = { false };
	int collisionChecksPerFrame;

#pragma region INITIALIZATION
	// initializes SDL, window, renderer, loads textures and sets console settings
	bool initializeEngine(const char* textureArr[], size_t textureCount)
	{
		// Initialize SDL
		if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
			std::cout << "Error initializing SDL: " << SDL_GetError() << std::endl;
			return false;
		}

		window = SDL_CreateWindow("Game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
		if (!window) {
			std::cout << "Error creating window: " << SDL_GetError() << std::endl;
			return false;
		}

		SDL_Renderer* render = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
		if (!render)
		{
			std::cout << "Error creating renderer: " << SDL_GetError() << std::endl;
			return false;
		}
		Rendering::setRenderer(render);
		SDL_SetRenderDrawBlendMode(render, SDL_BLENDMODE_BLEND);
		windowRect = SDL_Rect{ 0,0, WIDTH, HEIGHT };
		previousTicks = SDL_GetPerformanceCounter();

		// Hide cursor in console
		const HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
		CONSOLE_CURSOR_INFO cursorInfo;
		GetConsoleCursorInfo(out, &cursorInfo);
		cursorInfo.bVisible = false;
		SetConsoleCursorInfo(out, &cursorInfo);

		// lock size of console
		const HWND consoleWindow = GetConsoleWindow();
		SetWindowLong(consoleWindow, GWL_STYLE, GetWindowLong(consoleWindow, GWL_STYLE) & ~WS_MAXIMIZEBOX & ~WS_SIZEBOX);

		for (size_t i = 0; i < textureCount; i++)
		{
			SDL_Texture* texture = IMG_LoadTexture(render, textureArr[i]);
			textures.push_back(texture);
			SDL_SetTextureAlphaMod(texture, 255);
		}

		// makes rand() somewhat random
		srand(static_cast<UINT>(time(nullptr)));
		return true;
	}

#pragma endregion

#pragma region OBJECT_MANAGEMENT

	void createObject(Position position, Rotation rotation, Velocity velocity, float scaleOffset, SDL_Texture* texture, Tag tag)
	{
		// randomize texture if a texture wasn't included
		if (texture == nullptr)
			texture = textures[rand() % (textures.size())];

		// randomize color tint for each instance
		const Color tint{ rand() % 25 + 200 , rand() % 20 + 220 , rand() % 15 + 240 , 255 };
		const Appearance appearance(texture, scaleOffset, tint);

		positions.push_back(position);
		velocities.push_back(velocity);
		rotations.push_back(rotation);
		appearances.push_back(appearance);
		tags.push_back(tag);
		objectCount++;
	}

	void createObject(Position position, Rotation rotation, Velocity velocity, float scaleOffset, const char* texturePath, Tag tag)
	{
		SDL_Texture* texture = IMG_LoadTexture(Rendering::getRenderer(), texturePath);
		const Appearance appearance(texture, scaleOffset);

		const int result = SDL_QueryTexture(texture, NULL, NULL, NULL, NULL);
		if (result != 0)
		{
			std::cout << "Failed to load image at: " << texturePath << std::endl;
			return;
		}
		positions.push_back(position);
		velocities.push_back(velocity);
		rotations.push_back(rotation);
		appearances.push_back(appearance);
		tags.push_back(tag);
		objectCount++;
	}

	void removeObject(size_t index)
	{
		if (index < 0 || index > objectCount - 1 || index == playerIndex) return;

		positions.erase(positions.begin() + index);
		velocities.erase(velocities.begin() + index);
		rotations.erase(rotations.begin() + index);
		appearances.erase(appearances.begin() + index);
		tags.erase(tags.begin() + index);
		objectCount--;

		if (index < playerIndex) playerIndex--;
	}

	void splitObject(size_t index, Vector2 splitDirection)
	{
		int splits = 1;
		const float previousRadius = positions[index].radius;
		if (previousRadius > MIN_SIZE_WHOLENOTE)
		{
			positions[index].radius *= 0.3f;
			splits = 6;
		}
		else if (previousRadius > MIN_SIZE_TRIPPLESPLIT)
		{
			positions[index].radius *= 0.4f;
			splits = 2;
		}
		else
			positions[index].radius *= 0.5f;
		if (positions[index].radius < MIN_RADIUS)
		{
			// fade out and destroy object
			const auto convertedObject = ObjectPendingDeletion({ 0.33f}, positions[index], appearances[index], rotations[index].angle);
			objectsToDelete.push_back(convertedObject);
			removeObject(index);
			return;
		}
		float force = (rand() % 50 + 100) * 0.2f;
		Vector2 velocityDirection = splitDirection.perpendicularVector();
		velocities[index] = Velocity(velocityDirection.x * force, velocityDirection.y * force);
		positions[index].x += velocityDirection.x * previousRadius;
		positions[index].y += velocityDirection.y * previousRadius;
		rotations[index].angle = static_cast<float>(rand() % 360);

		for (int i = 0; i < splits; i++)
		{
			const float angleToRotate = 360.f / (splits + 1);
			rotateVector(velocityDirection.x, velocityDirection.y, angleToRotate);

			force = (rand() % 50 + 100) * 0.2f;
			createObject(positions[index], rotations[index], velocities[index], 0, appearances[index].texture);
			velocities[objectCount - 1] = Velocity(velocityDirection.x * force, velocityDirection.y * force);
			positions[objectCount - 1].x += velocityDirection.x * previousRadius;
			positions[objectCount - 1].y += velocityDirection.y * previousRadius;
			rotations[objectCount - 1].angle = static_cast<float>(rand() % 360);
		}
	}

	void createStar(float x, float y, float maxSize, Color color, int lifeTime, float elapsedTime)
	{
		starList.push_back(Star({ static_cast<float>(lifeTime), elapsedTime }, x, y, maxSize, color));
	}

	void createStartingStars(int count)
	{
		for (int i = 0; i < count; i++)
		{
			const auto randomX = static_cast<float>(rand() % (WIDTH - 100) + 50);
			const auto randomY = static_cast<float>(rand() % (HEIGHT - 150) + 50);
			const auto elapsedTime = static_cast<float>(rand() % 700) * 0.01f;
			Engine::createStar(randomX, randomY, 1, Color(200, 225, 255, 255), 10, elapsedTime);
		}
	}

	// Update elapsed lifetime of objects. Remove objects where elapsedLifeTime > totalLifeTime 
	void updateObjectsLifetime(float wholeNoteLength)
	{
		for (size_t i = starList.size(); i >= 1;) //reverse 
		{
			i--;
			starList[i].elapsedLifeTime += deltaTime;
			if (starList[i].elapsedLifeTime > wholeNoteLength * starList[i].totalLifeTime)
			{
				starList.erase(starList.begin() + i);
			}
		}
		for (size_t i = lineList.size(); i >= 1;) 
		{
			i--;
			lineList[i].elapsedLifeTime += deltaTime;
			if (lineList[i].elapsedLifeTime > lineList[i].totalLifeTime)
			{
				lineList.erase(lineList.begin() + i);
			}
		}
		for (size_t i = objectsToDelete.size(); i >= 1;) 
		{
			i--;
			objectsToDelete[i].elapsedLifeTime += deltaTime;
			if (objectsToDelete[i].elapsedLifeTime > objectsToDelete[i].totalLifeTime)
			{
				objectsToDelete.erase(objectsToDelete.begin() + i);
			}
		}
	}

	Position getPlayerPos()
	{
		return positions[playerIndex];
	}

	void updatePlayerTextureMod(Color color)
	{
		SDL_SetTextureColorMod(appearances[playerIndex].texture, color.r, color.g, color.b);
	}

	int getObjectCount()
	{
		return static_cast<int>(objectCount);
	}

	// adds a laser to the list of lasers
	// returns true if the shot was successful
	bool addLaser(Laser line, MusicData* music)
	{
		bool successfulShot = music->quarterNoteActive;
		line.color = successfulShot ? Color(0, 200, 255, 255) : Color(255, 0, 0, 255);
		
		for (size_t i = 0; i < objectCount; i++)
		{
			if (i == playerIndex) continue;

			if (pointCircleIntersect(line.x2, line.y2, positions[i]))
			{
				const bool isSplittable = appearances[i].tint.b > 0;

				successfulShot = isSplittable && (
					positions[i].radius > MIN_SIZE_WHOLENOTE && music->wholeNoteActive ||
					positions[i].radius > MIN_SIZE_HALFNOTE  && music->halfNoteActive  ||
					positions[i].radius < MIN_SIZE_HALFNOTE  && music->quarterNoteActive);

				if (successfulShot)
				{
					Vector2 splitDirection = Vector2(line.x1, line.y1) - Vector2(line.x2, line.y2);
					splitDirection.normalize();
					positions[i].x = line.x2;
					positions[i].y = line.y2;
					line.color = Color(0, 200, 255, 255);
					splitObject(i, splitDirection);
				}
				else
				{
					line.color = Color(255, 0, 0, 255);
					if (appearances[i].tint.b > 0)
					{
						appearances[i].tint = Color(150, 0, 0, 255);
						tags[i] = Tag::Unsplittable;
					}
				}
				break;
			}
		}
		lineList.push_back(line);
		return successfulShot;
	}

	void sortObjects()
	{
		if (objectCount <= 1) return;
		for (size_t i = 1; i < objectCount; i++)
		{
			size_t j = i;
			while (j > 0 && positions[j].x - positions[j].radius < positions[j - 1].x - positions[j - 1].radius)
			{
				std::swap(positions[j], positions[j - 1]);
				std::swap(velocities[j], velocities[j - 1]);
				std::swap(appearances[j], appearances[j - 1]);
				std::swap(rotations[j], rotations[j - 1]);
				std::swap(tags[j], tags[j - 1]);

				if (j == playerIndex) playerIndex--;
				else if (j - 1 == playerIndex) playerIndex++;

				j--;
			}
		}
	}

	void clearObjects()
	{
		for (size_t i = objectCount; i >= 1; i--)
		{
			removeObject(i - 1);
		}
	}
	// Handles movement, collision and de-penetration of all movable objects in the game
	void moveObjects()
	{
		float depenetrateX, depenetrateY;
		int count = 0;
		for (int i = 0; i < objectCount; i++)
		{
			positions[i].x += velocities[i].xVelocity * deltaTime;
			positions[i].y += velocities[i].yVelocity * deltaTime;

			for (int other = i + 1; other < objectCount; other++) // check collisions to the right
			{
				count++;
				// stop checking if the 'other' collider's left x coordinate is bigger than the original's right x coordinate
				if (positions[other].x - positions[other].radius > positions[i].x + positions[i].radius)
					break;
				if (circleIntersect(positions[i], positions[other], depenetrateX, depenetrateY))
				{
					// unsplittable objects can't be affected by other objects
					if (tags[i] == Tag::Unsplittable)
					{
						velocities[other] = velocities[i];
						positions[other].x += depenetrateX;
						positions[other].y += depenetrateY;
					}
					else if (tags[other] == Tag::Unsplittable)
					{
						velocities[i] = velocities[other];
						positions[i].x -= depenetrateX;
						positions[i].y -= depenetrateY;
					}
					// keep the velocity of the bigger collider
					else if (positions[i].radius > positions[other].radius)
					{
						velocities[other] = velocities[i];
						positions[other].x += depenetrateX;
						positions[other].y += depenetrateY;
					}
					else
					{
						velocities[i] = velocities[other];
						positions[i].x -= depenetrateX;
						positions[i].y -= depenetrateY;
					}
				}
			}

			if (tags[i] == Tag::Unsplittable)
				velocities[i].yVelocity += deltaTime * std::lerp(200.f, 5.f, velocities[i].yVelocity / 500);
			else
				velocities[i].yVelocity += deltaTime * std::lerp(25.f, 5.f, velocities[i].yVelocity / 50);
		}
		collisionChecksPerFrame = count;
	}

	// Checks for object collision with health line/laser
	// and also out of bounds positions
	void checkForObjectDestruction(Player* player)
	{
		for (size_t i = 0; i < objectCount; i++)
		{
			if (positions[i].y + positions[i].radius > HEALTH_LINE_HEIGHT && i != playerIndex)
			{
				auto convertedObject = ObjectPendingDeletion({ OBJECT_FADEOUT_TIME }, positions[i], appearances[i], rotations[i].angle);
				objectsToDelete.push_back(convertedObject);
				removeObject(i);
				player->takeDamage(2);
				continue;
			}
			if (positions[i].x + positions[i].radius <  0 || positions[i].x - positions[i].radius > WIDTH ||
				positions[i].y + positions[i].radius < -100 || positions[i].y - positions[i].radius > HEIGHT)
			{
				removeObject(i);
			}
		}
	}

	void rotateObjects()
	{
		for (size_t i = 0; i < objectCount; i++)
		{
			if (rotations[i].force == 0) continue;
			rotations[i].angle += rotations[i].force * deltaTime;

			if (rotations[i].angle > 360)
				rotations[i].angle = 0;
			if (rotations[i].angle < 0)
				rotations[i].angle = 360;
		}
	}
	void updateObjects(float wholeNoteLength)
	{
		sortObjects();
		moveObjects();
		rotateObjects();
		updateObjectsLifetime(wholeNoteLength);
	}
	void updatePlayerVelocity(float x, float y)
	{
		velocities[playerIndex].xVelocity = x;
		velocities[playerIndex].yVelocity = y;
	}

#pragma endregion

#pragma region RENDERING

	void drawEverything(MusicData* music, int playerHealth, bool gamePaused)
	{
		Rendering::drawBackground();

		if (!starList.empty())
			Rendering::drawStars(music, starList, elapsedTime);

		if (!lineList.empty())
			Rendering::drawLasers(lineList, positions, positions[playerIndex]);

		if (!objectsToDelete.empty())
			Rendering::drawObjectsFadingOut(objectsToDelete);

		Rendering::drawObjects(positions, appearances, rotations);
		Rendering::drawHealthLine(music, playerHealth);
		if (!gamePaused)
			Rendering::drawBeatCircles(music, positions, tags, playerIndex);
	}

	void unloadTextures()
	{
		// reversed for loop with unsigned int
		for (size_t i = textures.size(); i >= 1; i--)
		{
			SDL_DestroyTexture(textures[i-1]);
			textures.pop_back();
		}
		SDL_DestroyTexture(appearances[playerIndex].texture);
	}

#pragma endregion

#pragma region DELTATIME_RELATED

	float updateTicks()
	{
		const Uint64 currentTicks = SDL_GetPerformanceCounter();
		const Uint64 deltaTicks = currentTicks - previousTicks;
		deltaTime = static_cast<float>(deltaTicks) / SDL_GetPerformanceFrequency();
		previousTicks = currentTicks;
		elapsedTime += deltaTime;
		framerate = static_cast<int>(std::round(1 / deltaTime));

		return deltaTime;
	}

	void printTimeStats()
	{
		static const HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
		COORD coord = { static_cast<SHORT>(0), static_cast<SHORT>(0) };
		SetConsoleCursorPosition(hOut, coord);

		std::cout.flush();
		std::cout << " --------------------- \n";
		std::cout << "| Elapsed time: " << std::setprecision(1) << std::fixed << elapsedTime << "\n| \n";
		std::cout << "| FPS (capped): " << framerate << "    \n| \n";
		std::cout << "| Object count: " << objectCount << "  \n| \n";
		std::cout << "| Checks / object: " << collisionChecksPerFrame / objectCount << " \n --------------------- \n";



		for (int i = 0; i < 7; i++)
		{
			coord = { static_cast<SHORT>(22), static_cast<SHORT>(1 + i) };
			SetConsoleCursorPosition(hOut, coord);
			std::cout << "| ";
		}
		std::cout << "\n\n\n";
	}

	void delayNextFrame()
	{
		// Delays the current thread to keep the fps around 120 

		const Uint64 currentTicks = SDL_GetPerformanceCounter();
		deltaTime = static_cast<float>(currentTicks - previousTicks) / SDL_GetPerformanceFrequency();
		const int delay = static_cast<int>((0.00833 - deltaTime) * 1000);
		if (delay > 1)
			SDL_Delay(delay);

	}

#pragma endregion

#pragma region

	bool getKeyDown(int index)
	{
		return keys[index];
	}

	void updateKey(int index, bool value)
	{
		keys[index] = value;
		//std::cout << index << " was set to " << value << std::endl;
	}

	void resetKeys()
	{
		for (bool& key : keys)
		{
			key = false;
		}
	}

#pragma endregion KEY_MANAGEMENT

	void quit()
	{
		SDL_DestroyWindow(window);
		SDL_Quit();
	}
}

