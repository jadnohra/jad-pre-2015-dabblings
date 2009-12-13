#include "Math.h"

const Vector2D Vector2D::kZero(0.0f, 0.0f);

int IntersectLineCircle(const Vector2D& linePos, const Vector2D& lineDir, 
						const Vector2D& circlePos, float circleRadius,
						float& t, float & u)
{
	Vector2D f = linePos - circlePos;

	float a = Dot(lineDir, lineDir);
	float b = 2.0f*Dot(f, lineDir);
	float c = Dot(f, f) - circleRadius*circleRadius;

	float discriminant = b*b-4*a*c;
	if( discriminant < 0 )
	{
	  // no intersection
		return 0;
	}
	else
	{
	  // ray didn't totally miss sphere,
	  // so there is a solution to
	  // the equation.


	  discriminant = sqrtf( discriminant );
	  t = (-b - discriminant)/(2.0f*a);
	  u = (-b + discriminant)/(2.0f*a);
	  

	  return discriminant == 0.0f ? 1 : 2;
	}
}