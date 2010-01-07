#ifndef MATHINC_H
#define MATHINC_H

#include <math.h>
#include <stdlib.h>
#include <fstream>
#include <vector>
#include <algorithm>
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

	float GetMaxXForY(float y, float defaultX)
	{
		if (mPoints.size() == 0)
			return defaultX;

		size_t i;
		for (i=0;i<mPoints.size();++i)
		{
			if (mPoints[i].y > y)
			{
			}
			else
			{
				break;
			}
		}

		if (i >= mPoints.size())
		{
			return mPoints[mPoints.size()-1].x;
		} 
		else if (i > 0)
		{
			Vector2D* pPrev=&mPoints[i-1];
			Vector2D* pNext=&mPoints[i];

			float interp_factor = (y-pPrev->y) / (pNext->y-pPrev->y);

			return pPrev->x + (pNext->x-pPrev->x) * interp_factor;
		}

		return mPoints.size() == 0 ? defaultX : mPoints[0].x;
	}

	float GetXForY(float y, float defaultX)
	{
		Vector2D* pPrev=NULL;
		Vector2D* pNext=NULL;

		for (size_t i=0;i<mPoints.size();++i)
		{
			if (y == mPoints[i].y)
			{
				return mPoints[i].x;
			}
			else if (y > mPoints[i].y)
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
			return defaultX;

		if (pNext == NULL)
			return pPrev->x;

		float interp_factor = (y-pPrev->y) / (pNext->y-pPrev->y);

		return pPrev->x + (pNext->x-pPrev->x) * interp_factor;
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



class Poly2D
{
public:

	typedef std::vector<Vector2D> Points;
	Points points;

	void AddPoint(const Vector2D& pt)
	{
		points.push_back(pt);
	}

	void AddPointsConvex(const Vector2D& pt1, const Vector2D& pt2)
	{
		if (points.size() > 1)
		{
			const Vector2D& test_pt1 = points[points.size()-2];
			const Vector2D& test_pt2 = points[points.size()-1];

			float sign1 = SignedAngle(test_pt2-test_pt1, pt1-test_pt2);
			float sign2 = SignedAngle(pt1-test_pt2, pt2 - pt1);

			if ((sign1 >= 0.0f && sign2 >= 0.0f)
				|| (sign1 < 0.0f && sign2 < 0.0f))
			{
				AddPoint(pt1);
				AddPoint(pt2);
			}
			else
			{
				AddPoint(pt2);
				AddPoint(pt1);
			}
		}
		else
		{
			AddPoint(pt1);
			AddPoint(pt2);
		}
	}

	

	bool Contains(const Vector2D& pt)
	{
		int nvert = points.size();
		int i, j, c = 0;
		for (i = 0, j = nvert-1; i < nvert; j = i++) {
			if ( ((points[i].y>pt.y) != (points[j].y>pt.y)) &&
				(pt.x < (points[j].x-points[i].x) * (pt.y-points[i].y) / (points[j].y-points[i].y) + points[i].x) )
				c = !c;
		}
		return c;
	}

	// http://www.ecse.rpi.edu/Homepages/wrf/Research/Short_Notes/pnpoly.html#Almost%20Convex%20Polygons
	// works for general polys
	/*
	int pnpoly(int nvert, float *vertx, float *verty, float testx, float testy)
	{
		int i, j, c = 0;
		for (i = 0, j = nvert-1; i < nvert; j = i++) {
			if ( ((verty[i]>testy) != (verty[j]>testy)) &&
				(testx < (vertx[j]-vertx[i]) * (testy-verty[i]) / (verty[j]-verty[i]) + vertx[i]) )
				c = !c;
		}
		return c;
	}
	*/
};

class PolyPath2D
{
public:

	struct Portal
	{
		int index[2];
	};

	typedef std::vector<Poly2D> Polys;
	typedef std::vector<Portal> Portals;
	Polys polys;
	Portals portals;

	Poly2D& AddPoly()
	{
		polys.push_back(Poly2D());

		return polys.back();
	}

	void AddArc(int prevPortalIndex, const Vector2D& center, float width, float angle, int divCount)
	{
		if (polys.size() == 0)
			return;

		Vector2D startPt[2];
		Vector2D startRadius[2];

		startPt[0] = polys.back().points[prevPortalIndex];
		startPt[1] = polys.back().points[(prevPortalIndex+1)%polys.back().points.size()];

		startRadius[0] = startPt[0] - center;
		startRadius[1] = startPt[1] - center;

		int minPt = 0;

		if (startRadius[1].LengthSquared() < startRadius[0].LengthSquared())
			minPt = 1;

		int maxPt = (minPt+1)%2;

		Vector2D min_radius = startRadius[minPt];
		Vector2D max_radius = min_radius + (min_radius.Normalized() * width);

		float div_angle_inc = angle / (float) divCount;

		for (int i=0; i<divCount; ++i)
		{
			Vector2D div_min_radius = rotate(min_radius, (float)i * div_angle_inc);
			Vector2D div_max_radius = rotate(max_radius, (float)i * div_angle_inc);
			Vector2D next_div_min_radius = rotate(min_radius, (float)(i+1) * div_angle_inc);
			Vector2D next_div_max_radius = rotate(max_radius, (float)(i+1) * div_angle_inc);

			if (i == 0)
			{
				Poly2D& new_poly = AddPoly();
				
				new_poly.AddPoint(center+startRadius[maxPt]);
				new_poly.AddPoint(center+next_div_max_radius);
				new_poly.AddPoint(center+next_div_min_radius);
				new_poly.AddPoint(center+startRadius[minPt]);
			}
			else
			{
				Poly2D& new_poly = AddPoly();
				
				new_poly.AddPoint(center+div_max_radius);
				new_poly.AddPoint(center+next_div_max_radius);
				new_poly.AddPoint(center+next_div_min_radius);
				new_poly.AddPoint(center+div_min_radius);
			}
		}

	}

	void AddStraight(const Vector2D& start, const Vector2D& length, const Vector2D& width, int divCount, bool addConnection)
	{
		Vector2D unitLength = length * (1.0f / (float) divCount);
		Vector2D center = start + (unitLength * 0.5f);

		for (int i=0;i<divCount+1;++i)
		{
			Poly2D poly;
			BuildQuad(center, unitLength, width, poly);

			if (addConnection && i == 0 && polys.size()>0)
			{
				Poly2D conn;
				BuildConnectionQuad(polys.back(), poly, conn);
				polys.push_back(conn);
			}
			
			polys.push_back(poly);

			center = center + (unitLength);
		}
	}

	struct ManualAddContext
	{
		Poly2D poly1;
		Poly2D poly2;
	};

	void AddManualBuild(ManualAddContext& context, const Vector2D& pt)
	{
		if (polys.size() == 0)
		{
			if (context.poly1.points.size() < 2)
			{
				context.poly1.AddPoint(pt);
				return;
			}

			if (context.poly1.points.size() == 2)
			{
				if (context.poly2.points.size() == 0)
				{
					context.poly2.AddPoint(pt);
				}
				else
				{
					polys.push_back(Poly2D());
					polys.back().AddPointsConvex(context.poly1.points[0], context.poly1.points[1]);
					polys.back().AddPointsConvex(context.poly2.points[0], pt);
					context.poly1.points.clear();
					context.poly2.points.clear();
				}
			}
		}
		else
		{
			if (context.poly1.points.size() < 2)
			{
				context.poly1.AddPoint(pt);
			}

			if (context.poly1.points.size() == 2)
			{
				const Poly2D& prev = polys.back();

				Poly2D poly;
				poly.AddPointsConvex(prev.points[prev.points.size()-2], prev.points[prev.points.size()-1]);
				poly.AddPointsConvex(context.poly1.points[0], context.poly1.points[1]);
				polys.push_back(poly);
				context.poly1.points.clear();
			}
		}
	}

	void StartBuild(ManualAddContext* pContext=NULL)
	{
		polys.clear();
		portals.clear();

		if (pContext)
		{
			pContext->poly1.points.clear();
			pContext->poly2.points.clear();
		}
	}

	void EndBuild()
	{
		portals.clear();
		portals.reserve(polys.size());

		for (int i=0; i+1<polys.size(); ++i)
		{
			Poly2D& curr = polys[i];
			Poly2D& next = polys[i+1];

			portals.push_back(Portal());

			GetBestPortal(curr, next, portals.back());
		}
	}

	void BuildConnectionQuad(const Poly2D& curr, const Poly2D& next, Poly2D& conn)
	{
		Portal portal;

		GetBestPortal(curr, next, portal);

		conn.points.clear();
		conn.points.push_back(curr.points[portal.index[0]]);
		conn.points.push_back(curr.points[(portal.index[0]+1)%curr.points.size()]);

		Vector2D pt1=next.points[portal.index[1]];
		Vector2D pt2=next.points[(portal.index[1]+1)%next.points.size()];

		conn.AddPointsConvex(pt1, pt2);
	}

	struct Info
	{
		int index_curr;
		int index_next;
		float dist_sq;

		inline bool operator<(const Info& comp) const
		{
			return dist_sq < comp.dist_sq;
		}
	};

	void GetBestPortal(const Poly2D& curr, const Poly2D& next, Portal& portal)
	{
		typedef std::vector<Info> Infos;
		Infos infos;

		infos.reserve(curr.points.size()*next.points.size());

		for (int i=0; i<curr.points.size(); ++i)
		{
			for (int j=0; j<next.points.size(); ++j)
			{
				infos.push_back(Info());
				infos.back().index_curr = i;
				infos.back().index_next = j;
				infos.back().dist_sq = DistanceSquared(curr.points[i], next.points[j]);
			}
		}

		std::sort(infos.begin(), infos.end());

		int curr_pt_index_1 = infos[0].index_curr;
		int curr_pt_index_2 = -1;
		int next_pt_index_1 = infos[0].index_next;
		int next_pt_index_2 = -1;

		for (int i=1;i<infos.size() && ((curr_pt_index_2 == -1)||next_pt_index_2 == -1); ++i)
		{
			if (curr_pt_index_2 == -1)
			{
				if (infos[i].index_curr == ((curr_pt_index_1+1)%curr.points.size())
					|| infos[i].index_curr == ((curr_pt_index_1+curr.points.size()-1)%curr.points.size()))
				{
					curr_pt_index_2 = infos[i].index_curr;

					if (curr_pt_index_2 == curr_pt_index_1)
						curr_pt_index_2 = -1;
				}
			}

			if (next_pt_index_2 == -1)
			{
				if (infos[i].index_next == ((next_pt_index_1+1)%next.points.size())
					|| infos[i].index_next == ((next_pt_index_1+next.points.size()-1)%next.points.size()))
				{
					next_pt_index_2 = infos[i].index_next;

					if (next_pt_index_2 == next_pt_index_1)
						next_pt_index_2 = -1;
				}
			}
		}

		if (curr_pt_index_2 == -1 || curr_pt_index_1+1 == curr_pt_index_2)
			portal.index[0] = curr_pt_index_1;
		else
			portal.index[0] = curr_pt_index_2;


		if (next_pt_index_2 == -1 || next_pt_index_1+1 == next_pt_index_2)
			portal.index[1] = next_pt_index_1;
		else
			portal.index[1] = next_pt_index_2;
	}

	void BuildQuad(const Vector2D& center, const Vector2D& length, const Vector2D& width, Poly2D& poly)
	{
		poly.points.clear();
		poly.points.push_back(center - (width*0.5f) - (length*0.5f));
		poly.points.push_back(center - (width*0.5f) + (length*0.5f));
		poly.points.push_back(center + (width*0.5f) + (length*0.5f));
		poly.points.push_back(center + (width*0.5f) - (length*0.5f));
	}


	void Serialize(const char* path)
	{
		std::ofstream file;
		file.open(path);

		size_t sz=polys.size();
		file.write((const char*)&sz, sizeof(size_t));

		printf("%d polys\n", sz);

		sz=portals.size();
		file.write((const char*)&sz, sizeof(size_t));

		printf("%d portals\n", sz);

		for (size_t i=0;i<polys.size();++i)
		{
			sz=polys[i].points.size();
			file.write((const char*)&sz, sizeof(size_t));

			printf("%d points\n", sz);

			for (size_t j=0;j<polys[i].points.size();++j)
			{
				printf("%f, %f\n", polys[i].points[j].x, polys[i].points[j].y);

				file.write((const char*)&(polys[i].points[j].x), sizeof(float));
				file.write((const char*)&(polys[i].points[j].y), sizeof(float));
			}
		}

			

		for (size_t i=0;i<portals.size();++i)
		{
			printf("%d, %d\n", portals[i].index[0], portals[i].index[1]);

			file.write((const char*)&(portals[i].index[0]), sizeof(int));
			file.write((const char*)&(portals[i].index[1]), sizeof(int));
		}

		file.close();
	}

	bool Deserialize(const char* path)
	{
		std::ifstream file;

		polys.clear();
		portals.clear();

		file.open(path);

		if (file.is_open())
		{

			size_t sz1=0;
			file.read((char*)&sz1, sizeof(size_t));
			size_t sz3=0;
			file.read((char*)&sz3, sizeof(size_t));

			printf("%d polys\n", sz1);

			for (size_t i=0;i<sz1;++i)
			{
				size_t sz2=0;
				file.read((char*)&sz2, sizeof(size_t));

				printf("%d points\n", sz2);

				polys.push_back(Poly2D());

				for (size_t j=0;j<sz2;++j)
				{
					float x,y;

					file.read((char*)&x, sizeof(float));
					file.read((char*)&y, sizeof(float));

					printf("%f, %f\n", x, y);

					polys.back().AddPoint(Vector2D(x, y));
				}
			}

			

			printf("%d portals\n", sz3);

			for (size_t i=0;i<sz3;++i)
			{
				int x,y;

				file.read((char*)&x, sizeof(int));
				file.read((char*)&y, sizeof(int));

				printf("%d, %d\n", x, y);

				portals.push_back(Portal());
				portals.back().index[0] = x;
				portals.back().index[1] = y;
			}

			return true;
		}

		return false;
	}
};


#endif