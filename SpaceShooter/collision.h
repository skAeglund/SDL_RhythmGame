#pragma once
#include "gameObject.h"
#include "vector2.h"

typedef GameObject::Position Circle;

namespace Collision
{
	using namespace Vector2D;

	bool circleIntersect(Circle a, Circle b, float& depenetrateX, float& depenetrateY);
	bool pointCircleIntersect(float x, float y, const Circle& circle);
}