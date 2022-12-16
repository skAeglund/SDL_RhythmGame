#pragma once
#include "engine.h"
#include "player.h"
#include "musicManager.h"

/// <summary>
/// I will restructure things from engine to here...
/// </summary>

//struct Position { float x; float y; float radius = 50; };
//struct Velocity { float xVelocity; float yVelocity; };
//struct Rotation { float force; float angle; };
//struct Appearance { SDL_Texture* texture; float scaleOffset = 0; SDL_Color tint = SDL_Color(255, 255, 255, 0); };
//struct ObjectPendingDeletion { Position position; Appearance appearance; float elapsedFadeOutTime; float angle; };
//
//struct Game
//{
//public:
//	Player player;
//private:
//	MovableObjects objects;
//
//	std::vector<Line> lineList;
//	std::vector<ObjectPendingDeletion> objectsToDelete;
//};
//
//struct MovableObjects
//{
//	std::vector<Position> positions;
//	std::vector<Velocity> velocities;
//	std::vector<Rotation> rotations;
//	std::vector<Appearance> textures;
//	std::vector<Tag> tags;
//};