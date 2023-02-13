#pragma once
#define PI 3.14159265359f
#include <cmath>

/**
 * \brief A collection of static easing functions. Handles floats within 0-1 range. 
 */
class Ease
{
public:
	static float In(float x, int power)
	{
		return std::powf(x, static_cast<float>(power));
	}

	static float Out(float x, int power)
	{
		return 1 - std::powf(1 - x, static_cast<float>(power));
	}

	static float InOutSine(float x)
	{
		return 1 - cos((x * PI) / 2);
	}
};