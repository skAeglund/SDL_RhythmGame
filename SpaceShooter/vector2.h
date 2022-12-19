#pragma once
#include <iostream>
#define PI 3.14159265f

namespace Vector2D
{
	struct Vector2 {
		float x;
		float y;
		Vector2(float x = 0.f, float y = 0.f);
		float length();
		void clamp(float min, float max);
		void normalize();
		Vector2 normalized();
		Vector2 perpendicularVectorClockwise();
		Vector2 perpendicularVectorCounterClockwise();

		Vector2& operator*=(float rhs)
		{
			this->x *= rhs;
			this->y *= rhs;
			return *this;
		}
		Vector2& operator-=(const Vector2& rhs)
		{
			this->x -= rhs.x;
			this->y -= rhs.y;
			return *this;
		}
		Vector2& operator+=(const Vector2& rhs)
		{
			this->x += rhs.x;
			this->y += rhs.y;
			return *this;
		}
		Vector2 operator*(const float& rhs) const
		{
			return Vector2(x * rhs, y * rhs);
		}
		Vector2 operator-(const Vector2& rhs) const
		{
			return Vector2(x - rhs.x, y - rhs.y);
		}
		Vector2 operator+(const Vector2& rhs) const
		{
			return Vector2(x + rhs.x, y + rhs.y);
		}
	};


	float magnitude(float x, float y);

	float distance(float x1, float y1, float x2, float y2);

	Vector2 unitDirection(const Vector2& a, const Vector2& b);

	Vector2 unitDirection(float x1, float y1, float x2, float y2);

	void rotateVector(float& x, float& y, const float angle);

	void makePerpendicularClockwise(float& x, float& y);

	void normalize(float& x, float& y);
}

