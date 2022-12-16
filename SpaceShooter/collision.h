#pragma once
#include "engine.h"
#include "vector2.h"

enum class Button { start, quit, none };
Button checkButtonAtPoint(int x, int y);

struct Circle
{
	float x;
	float y;
	float radius;
};

bool circleIntersect(float x1, float y1, float r1, float x2, float y2, float r2, Vector2& depenetrationVector);
bool circleIntersect(float x1, float y1, float r1, float x2, float y2, float r2, float& depenetrateX, float& depenetrateY);
//bool lineCircleIntersect(const Line& line, const Circle& circle);


bool pointCircleIntersect(float x, float y, const Circle& circle);
