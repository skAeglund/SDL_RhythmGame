#include "vector2.h"

namespace Vector2D
{
	Vector2::Vector2(float x, float y)
	{
		this->x = x;
		this->y = y;
	}

	float Vector2::length()
	{
		return sqrt(x * x + y * y);
	}

	void Vector2::clamp(float min, float max)
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
		float previousLength = length();
		if (previousLength == 0) return;

		x /= previousLength;
		y /= previousLength;
	}

	Vector2 Vector2::normalized()
	{
		Vector2 copy = *this;
		copy.normalize();
		return copy;
	}
	Vector2 Vector2::perpendicularVectorClockwise()
	{
		return Vector2(this->y, -this->x);
	}
	Vector2 Vector2::perpendicularVectorCounterClockwise()
	{
		return Vector2(-this->y, this->x);
	}

	Vector2 unitDirection(const Vector2& a, const Vector2& b)
	{
		Vector2 direction = b - a;
		return direction.normalized();
	}
	Vector2 unitDirection(float x1, float y1, float x2, float y2)
	{
		float dX = x2 - x1;
		float dY = y2 - y1;
		return Vector2(dX, dY).normalized();
	}
	void makePerpendicularClockwise(float& x, float& y)
	{
		float pX = x;
		float pY = y;
		x = pY;
		y = -pX;
	}
	void rotateVector(float& x, float& y, const float angle)
	{
		float radians = (angle * PI) / 180; // convert degrees to radians

		float cosine = cos(radians);
		float sine = sin(radians);

		float rotatedX = x * cosine - y * sine;
		float rotatedY = x * sine + y * cosine;
		x = rotatedX;
		y = rotatedY;
	}

	void normalize(float& x, float& y)
	{
		float previousLength = sqrt(x * x + y * y);
		if (previousLength <= 0) return;

		x /= previousLength;
		y /= previousLength;
	}
	float distance(float x1, float y1, float x2, float y2)
	{
		return sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));
	}
	float magnitude(float x, float y)
	{
		return sqrt(x * x + y * y);
	}
}