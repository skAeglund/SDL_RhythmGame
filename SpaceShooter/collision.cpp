#include "collision.h"
#include <cmath>

#include "gameObject.h"


namespace Collision
{
	bool circleIntersect(Circle a, Circle b, float& depenetrateX, float& depenetrateY)
	{
		const float dx = b.x - a.x;
		const float dy = b.y - a.y;

		const float dist = sqrt(dx * dx + dy * dy);

		const float radiusSum = a.radius + b.radius;
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
		const float dx = x - circle.x;
		const float dy = y - circle.y;

		const float dist = sqrt(dx * dx + dy * dy);

		return dist < circle.radius;
	}

}
