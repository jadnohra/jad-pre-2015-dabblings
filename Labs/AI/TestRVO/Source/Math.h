#ifndef MATH_H
#define MATH_H

#include <math.h>
#include <stdlib.h>
#include "Box2D.h"

#define MATH_PIf 3.14159265f

struct Vector2D : public b2Vec2
{
	Vector2D(float v0 = 0.0f, float v1 = 0.0f)
	{
		x = v0;
		y = v1;
	}

	float& operator[](int index) { return index == 0 ? x : y; }

	Vector2D operator*(float mul) const
	{
		return Vector2D(x * mul, y * mul);
	}

	Vector2D operator+(float mul) const
	{
		return Vector2D(x + mul, y + mul);
	}

	Vector2D operator-(float mul) const
	{
		return Vector2D(x - mul, y - mul);
	}

	Vector2D operator*(const Vector2D& mul) const
	{
		return Vector2D(x * mul.x, y * mul.y);
	}

	Vector2D operator+(const Vector2D& mul) const
	{
		return Vector2D(x + mul.x, y + mul.y);
	}

	Vector2D operator-(const Vector2D& mul) const
	{
		return Vector2D(x - mul.x, y - mul.y);
	}

	Vector2D& operator*=(const Vector2D& mul) 
	{
		return (*this = Vector2D(x * mul.x, y * mul.y));
	}

	Vector2D& operator+=(const Vector2D& mul) 
	{
		return (*this = Vector2D(x + mul.x, y + mul.y));
	}

	Vector2D& operator-=(const Vector2D& mul) 
	{
		return (*this = Vector2D(x - mul.x, y - mul.y));
	}

	bool operator==(const Vector2D& comp) const
	{
		return x == comp.x && y == comp.y;
	}

	bool operator!=(const Vector2D& comp) const
	{
		return !(*this == comp);
	}

	void Zero()
	{
		*this = kZero;
	}

	float Length() const
	{
		return sqrtf(x*x + y*y);
	}

	void Normalize()
	{
		*this = Normalized();
	}

	Vector2D Normalized() const
	{
		float length = Length();

		if (length > 0.0f)
			return (*this * (1.0f / length));
		else
			return kZero;
	}

	static const Vector2D kZero;
};

struct Circle
{
	Vector2D pos;
	float radius;
};

inline float Dot(const Vector2D& p1, const Vector2D& p2)
{
	return p1.x * p2.x + p1.y * p2.y;
}

inline float Distance(const Vector2D& p1, const Vector2D& p2)
{
	Vector2D d = p2 - p1;

	return sqrtf(Dot(d, d));
}

inline float Randf(float min, float max)
{
	return ( 0.1f + (float)rand()/0x7fff )* (max-min) + min;
}


int IntersectLineCircle(const Vector2D& linePos, const Vector2D& lineDir, 
						const Vector2D& circlePos, float circleRadius,
						float& t, float & u);


inline Vector2D rotate(const Vector2D& v, float rads)
{
	return Vector2D(cosf(rads) * v.x - sinf(rads) * v.y, cosf(rads) * v.y + sinf(rads) * v.x);
}

#endif