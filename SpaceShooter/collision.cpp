#include "collision.h"
#include <algorithm>
#include <cmath>
#include <map>

namespace Collision
{ 
std::map <Button, SDL_Rect> buttons
{
	{Button::start, SDL_Rect{430,625,420,140}},
	{Button::quit, SDL_Rect{1028,625,420,140}}
};

Button checkButtonAtPoint(int x, int y)
{
	SDL_Point point{ x,y };
	for (auto& button : buttons)
	{
		if (SDL_PointInRect(&point, &button.second))
		{
			return button.first;
		}
	}
	return Button::none;
}

bool circleIntersect(float x1, float y1, float r1, float x2, float y2, float r2, Vector2& depenetrationVector)
{
	float dx = x2 - x1;
	float dy = y2 - y1;

	float dist = sqrt(dx * dx + dy * dy);

	float radiusSum = r1 + r2;
	if (dist < radiusSum)
	{
		depenetrationVector = Vector2(dx, dy);
		depenetrationVector.normalize();
		depenetrationVector *= radiusSum - dist;
	}
	return dist < radiusSum;
}
bool circleIntersect(float x1, float y1, float r1, float x2, float y2, float r2, float& depenetrateX, float& depenetrateY)
{
	float dx = x2 - x1;
	float dy = y2 - y1;

	float dist = sqrt(dx * dx + dy * dy);

	float radiusSum = r1 + r2;
	if (dist < radiusSum)
	{
		depenetrateX = dx;
		depenetrateY = dy;
		normalize(depenetrateX, depenetrateY);
		depenetrateX *= radiusSum - dist;
		depenetrateY *= radiusSum - dist;
	}
	return dist < radiusSum;
}

bool pointCircleIntersect(float x, float y, const Circle& circle)
{
	float dx = x - circle.x;
	float dy = y - circle.y;

	float dist = sqrt(dx * dx + dy * dy);

	return dist < circle.radius;
}

}