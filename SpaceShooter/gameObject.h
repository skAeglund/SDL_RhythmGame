#pragma once
#include <SDL.h>
#include <Windows.h>

/**
 * \brief A collection of all attributes and structs related to game objects.
 */
namespace GameObject
{
	// Objects with a radius above this will be connected with whole notes (beat circles)
	#define MIN_SIZE_WHOLENOTE 45

	// Objects with a radius above this will be connected with half notes (beat circles) - below this = quarter notes
	#define MIN_SIZE_HALFNOTE 20

	// Size needed for an object to be splitted into 3
	#define MIN_SIZE_TRIPPLESPLIT 32

	// An object with radius less than this will be destroyed when hit with laser
	#define MIN_RADIUS 10

	// The time it takes for an object to fade out before getting destroyed
	#define OBJECT_FADEOUT_TIME 1.5f

	#define LASER_DEFAULT_LIFETIME 0.7f

	// To shorten multiplication of colors and avoid having to cast to UINT8 everywhere.
	struct Color
	{
		UINT8 r, g, b, a;

		Color(int r, int g, int b, int a) : r(static_cast<UINT8>(r)), g(static_cast<UINT8>(g)),
											b(static_cast<UINT8>(b)), a(static_cast<UINT8>(a)) {}

		Color(float r, float g, float b, float a) : r(static_cast<UINT8>(r)), g(static_cast<UINT8>(g)),
													b(static_cast<UINT8>(b)), a(static_cast<UINT8>(a)) {}

		// Initialize by multiplying a previous color
		Color(Color color, float multiplier, float alphaMultiplier = 1.f) : r(static_cast<UINT8>(static_cast<float>(color.r)* multiplier)),
											   g(static_cast<UINT8>(static_cast<float>(color.g) * multiplier)),
											   b(static_cast<UINT8>(static_cast<float>(color.b) * multiplier)),
											   a(static_cast<UINT8>(static_cast<float>(color.a) * alphaMultiplier)) {}

		Color multiplied(float multiplier, float alphaMultiplier = 1.f) { return { *this, multiplier, alphaMultiplier}; };
	};

	// These are the attributes of all movable objects
	struct Position { float x; float y; float radius = 50; }; // Includes radius since it's used together with position most of the time
	struct Velocity { float xVelocity; float yVelocity; };
	struct Rotation { float force; float angle; };
	struct Appearance { SDL_Texture* texture; float scaleOffset = 0; Color tint = Color(255, 255, 255, 0); };
	enum class Tag { Asteroid, Unsplittable, Player, none };

	// An object with a set lifetime, gets deleted when <elapsedLifeTime> is greater than <totalLifetime>
	struct LifeTimeObject { float totalLifeTime; float elapsedLifeTime = 0.f; };

	// Before destruction, any movable object will be converted into this struct, allowing the object to fade out without caring about collisions etc.
	struct ObjectPendingDeletion : LifeTimeObject { Position position; Appearance appearance; float angle; };

	struct Laser : LifeTimeObject { float x1; float y1; float x2; float y2; Color color = Color(0, 255, 255, 255); };

	// This doesn't contain the actual geometry of the star. That's handled inside Rendering::drawStar() for now. 
	struct Star : LifeTimeObject { float x; float y; float maxSize; Color color; };

}


