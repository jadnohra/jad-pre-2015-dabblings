#ifndef MATHINC_H
#define MATHINC_H

#include <math.h>
#include <stdlib.h>
#include <fstream>
#include <vector>
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

	Circle(const Vector2D& inPos, float inRadius)
	:	pos(inPos)
	,	radius(inRadius)
	{
	}
};

struct Quad2D
{
	Vector2D points[4];

	bool Contains(const Vector2D& pt)
	{
		return true;
	}

	int IntersectsLine(const Vector2D& pt, const Vector2D& dir, float& r, float& s)
	{
		return 0;
	}
};

inline float Dot(const Vector2D& p1, const Vector2D& p2)
{
	return p1.x * p2.x + p1.y * p2.y;
}

inline float DistanceSquared(const Vector2D& p1, const Vector2D& p2)
{
	Vector2D d = p2 - p1;

	return Dot(d, d);
}

inline float Distance(const Vector2D& p1, const Vector2D& p2)
{
	return sqrtf(DistanceSquared(p1, p2));
}

inline float Randf(float min, float max)
{
	return ( 0.1f + (float)rand()/0x7fff )* (max-min) + min;
}


int IntersectLineCircle(const Vector2D& linePos, const Vector2D& lineDir, 
						const Vector2D& circlePos, float circleRadius,
						float& t, float & u);


inline bool IntersectLines(const Vector2D& seg1Pt1, const Vector2D& seg1Pt2, 
							const Vector2D& seg2Pt1, const Vector2D& seg2Pt2, float& r, float& s)
{
	float denom = (seg1Pt2.x-seg1Pt1.x)*(seg2Pt2.y-seg2Pt1.y)-(seg1Pt2.y-seg1Pt1.y)*(seg2Pt2.x-seg2Pt1.x);
	
	if (denom == 0.0f)
		return false;

	r = ((seg1Pt1.y-seg2Pt1.y)*(seg2Pt2.x-seg2Pt1.x)-(seg1Pt1.x-seg2Pt1.x)*(seg2Pt2.y-seg2Pt1.y)) / denom;
	s = ((seg1Pt1.y-seg2Pt1.y)*(seg1Pt2.x-seg1Pt1.x)-(seg1Pt1.x-seg2Pt1.x)*(seg1Pt2.y-seg1Pt1.y)) / denom;
	
	return true;
}


inline bool IntersectLineSegment(const Vector2D& line1Pt1, const Vector2D& line1Pt2, 
								 const Vector2D& seg2Pt1, const Vector2D& seg2Pt2, Vector2D& inters)
{

	float t, u;

	if (IntersectLines(line1Pt1, line1Pt2, seg2Pt1, seg2Pt2, t, u))
	{
		if (u >= 0.0f && u <= 1.0f)
		{
			inters = line1Pt1+((line1Pt2-line1Pt1)*t);
			return true;
		}
	}

	return false;
}


inline bool IntersectSegments(const Vector2D& seg1Pt1, const Vector2D& seg1Pt2, 
							  const Vector2D& seg2Pt1, const Vector2D& seg2Pt2, Vector2D& inters)
{
	
	float t, u;

	if (IntersectLines(seg1Pt1, seg1Pt2, seg2Pt1, seg2Pt2, t, u))
	{
		if ((t >= 0.0f && t <= 1.0f) && (u >= 0.0f && u <= 1.0f))
		{
			inters = seg1Pt1+((seg1Pt2-seg1Pt1)*t);
			return true;
		}
	}

	return false;
}

inline float DistancePointLineSquared(const Vector2D& linePos, const Vector2D& lineDir, 
									const Vector2D& point, float* pU = NULL)
{
	Vector2D diff = point - linePos;
	float u = Dot(diff, lineDir) / Dot(lineDir, lineDir);
	Vector2D closestPt = linePos + (lineDir * u);

	if (pU)
		*pU = u;

	return DistanceSquared(closestPt, point);
}

inline float DistancePointSeqSquared(const Vector2D& segPos1, const Vector2D& segPos2, 
									const Vector2D& point, float* pU = NULL)
{
	Vector2D diff = point - segPos1;
	Vector2D lineDir = segPos2-segPos1;
	float u = Dot(diff, lineDir) / Dot(lineDir, lineDir);
	
	if (u < 0.0f)
		u=0.0f;
	if (u>1.0f)
		u=1.0f;
	
	Vector2D closestPt = segPos1 + (lineDir * u);

	if (pU)
		*pU = u;

	return DistanceSquared(closestPt, point);
}

inline Vector2D rotate(const Vector2D& v, float rads)
{
	return Vector2D(cosf(rads) * v.x - sinf(rads) * v.y, cosf(rads) * v.y + sinf(rads) * v.x);
}

inline Vector2D rotate90(const Vector2D& v)
{
	return Vector2D(-v.y,v.x);
}

inline int CreateBoxQuad(const b2AABB& inBox, Vector2D* outQuad)
{
	int point_index = 0;
	int unique_point_count;

	outQuad[point_index++] = Vector2D(inBox.lowerBound.x, inBox.lowerBound.y);
	outQuad[point_index++] = Vector2D(inBox.lowerBound.x, inBox.upperBound.y);
	outQuad[point_index++] = Vector2D(inBox.upperBound.x, inBox.upperBound.y);
	outQuad[point_index++] = Vector2D(inBox.upperBound.x, inBox.lowerBound.y);

	unique_point_count = 4;
	if (inBox.lowerBound.x == inBox.upperBound.x)
		unique_point_count /= 2;

	if (inBox.lowerBound.y == inBox.upperBound.y)
		unique_point_count /= 2;
	
	return unique_point_count;
}


inline float GetPointSideOfLine(const Vector2D &linePos, const Vector2D& lineDir, const Vector2D& point)
{
	Vector2D normal = rotate90(lineDir);

	return Dot(normal, point - linePos);
}


inline float SignedAngle(const Vector2D & v1, const Vector2D & v2)
{
	float perpDot = v1.x * v2.y - v1.y * v2.x;
	return (float) atan2f(perpDot, Dot(v1, v2));
}


struct ResponseCurve
{
	typedef std::vector<Vector2D> Points;
	Points mPoints;

	void Clear()
	{
		mPoints.clear();
	}

	void Add(float x, float y)
	{
		mPoints.push_back(Vector2D(x, y));
	}

	float Get(float x, float defaultY)
	{
		Vector2D* pPrev=NULL;
		Vector2D* pNext=NULL;

		for (size_t i=0;i<mPoints.size();++i)
		{
			if (x == mPoints[i].x)
			{
				return mPoints[i].y;
			}
			else if (x > mPoints[i].x)
			{
				pPrev = &mPoints[i];
			}
			else
			{
				pNext = &mPoints[i];
				break;
			}
		}

		if (pPrev == NULL)
			return defaultY;

		if (pNext == NULL)
			return pPrev->y;

		float interp_factor = (x-pPrev->x) / (pNext->x-pPrev->x);

		return pPrev->y + (pNext->y-pPrev->y) * interp_factor;
	}

	void Serialize(const char* path)
	{
		std::ofstream file;
		file.open(path);

		size_t sz=mPoints.size();
		file.write((const char*)&sz, sizeof(size_t));

		for (size_t i=0;i<mPoints.size();++i)
		{
			file.write((const char*)&(mPoints[i].x), sizeof(float));
			file.write((const char*)&(mPoints[i].y), sizeof(float));
		}
	}

	bool Deserialize(const char* path)
	{
		std::ifstream file;

		file.open(path);

		if (file.is_open())
		{

			size_t sz=0;
			file.read((char*)&sz, sizeof(size_t));

			for (size_t i=0;i<sz;++i)
			{
				float x,y;

				file.read((char*)&x, sizeof(float));
				file.read((char*)&y, sizeof(float));

				Add(x, y);
			}

			return true;
		}

		return false;
	}
};

#endif