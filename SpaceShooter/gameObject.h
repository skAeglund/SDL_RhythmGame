#pragma once
#include <SDL.h>

namespace GameObject
{
	// objects with a radius above this will be timed with wholenotes (beat circles)
	#define MIN_SIZE_WHOLENOTE 45

	// objects with a radius above this and below <MIN_SIZE_WHOLENOTE> are timed with halfnotes (beat circles)
	// objects with a radius below this are timed with quarternotes
	#define MIN_SIZE_HALFNOTE 20

	// size needed for an object to be splitted into 3
	#define MIN_SIZE_TRIPPLESPLIT 32

	// an object with radius less than this will be destroyed when hit with laser
	#define MIN_RADIUS 10

	// these are the attributes of all movable objects
	struct Position { float x; float y; float radius = 50; };
	struct Velocity { float xVelocity; float yVelocity; };
	struct Rotation { float force; float angle; };
	struct Appearance { SDL_Texture* texture; float scaleOffset = 0; SDL_Color tint = SDL_Color(255, 255, 255, 0); };
	enum class Tag { Asteroid, Unsplittable, Player, none };

	struct ObjectPendingDeletion { Position position; Appearance appearance; float elapsedFadeOutTime; float angle; };

	struct Laser { float x1; float y1; float x2; float y2; float elapsedTime = 0.f; SDL_Color color = SDL_Color(0, 255, 255, 255); }; //todo: move
}


