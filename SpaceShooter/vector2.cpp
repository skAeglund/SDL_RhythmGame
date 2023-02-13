#include "vector2.h"

#include <cmath>

#include "easingFunctions.h"

namespace Vector2D
{
	Vector2::Vector2(float x, float y) : x(x), y(y) {}

	float Vector2::length() const
	{
		return sqrt(x * x + y * y);
	}

	void Vector2::clamp(const float min, const float max)
	{
		if (length() < min)
		{
			normalize();
			x *= min;
			y *= min;
		}
		else if (length() > max)
		{
			normalize();
			x *= max;
			y *= max;
		}
	}
	void Vector2::normalize()
	{
		const float previousLength = length();
		if (previousLength == 0.f) return;

		x /= previousLength;
		y /= previousLength;
	}

	Vector2 Vector2::normalized() const
	{
		Vector2 copy = *this;
		copy.normalize();
		return copy;
	}
	// Clockwise rotation 90 degrees
	Vector2 Vector2::perpendicularVector() const
	{
		return { this->y, -this->x };
	}
	// Overloaded: Counter-clockwise rotation 90 degrees
	Vector2 Vector2::perpendicularVector(bool counterClock) const
	{
		return { -this->y, this->x };
	}

	Vector2 unitDirection(const Vector2& a, const Vector2& b)
	{
		Vector2 direction = b - a;
		return direction.normalized();
	}
	Vector2 unitDirection(float x1, float y1, float x2, float y2)
	{
		const float dX = x2 - x1;
		const float dY = y2 - y1;
		return Vector2(dX, dY).normalized();
	}
	void makePerpendicularClockwise(float& x, float& y)
	{
		const float pX = x;
		const float pY = y;
		x = pY;
		y = -pX;
	}
	void rotateVector(float& x, float& y, const float angle)
	{
		const float radians = (angle * PI) / 180; // convert degrees to radians

		const float cosine = cos(radians);
		const float sine = sin(radians);

		const float rotatedX = x * cosine - y * sine;
		const float rotatedY = x * sine + y * cosine;
		x = rotatedX;
		y = rotatedY;
	}

	void normalize(float& x, float& y)
	{
		const float previousLength = sqrt(x * x + y * y);
		if (previousLength <= 0) return;

		x /= previousLength;
		y /= previousLength;
	}
	float distance(float x1, float y1, float x2, float y2)
	{
		return sqrtf(powf(x2 - x1, 2) + powf(y2 - y1, 2));
	}
	float magnitude(float x, float y)
	{
		return sqrtf(x * x + y * y);
	}

	void rotatePoint(float& x, float& y, float centerX, float centerY, float angleInDegrees)
	{
		const float angleInRadians = angleInDegrees * (PI / 180);
		const float cosTheta = cos(angleInRadians);
		const float sinTheta = sin(angleInRadians);

		const float oldX = x;
		const float oldY = y;

		x = (cosTheta * (oldX - centerX) - sinTheta * (oldY - centerY) + centerX);

		y = (sinTheta * (oldX - centerX) + cosTheta * (oldY - centerY) + centerY);
	}
}
