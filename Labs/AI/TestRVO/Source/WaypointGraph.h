#ifndef WAYPONITGRAPH_H
#define WAYPONITGRAPH_H

#include <vector>
#include "Math.h"
#include "World.h"

struct Waypoint
{
	Vector2D pos;
	float radius;
};

template <typename NodeT>
class NodeGraph
{
public:

	struct Node : NodeT
	{
		int linksIndex;
		int nodeIndex;
	};

	typedef std::vector<int> NodeLinks;
	typedef std::vector<NodeLinks> Links;
	typedef std::vector<Node> Nodes;

	Nodes mNodes;
	Links mLinks;

	Node& AddNode()
	{
		int index = (int) mNodes.size();
		mNodes.push_back(Node());
		mNodes.back().nodeIndex = index;
		mNodes.back().linksIndex = -1;
		return mNodes.back();
	}


	void LinkNode(Node& from, Node& to)
	{
		if (from.linksIndex == -1)
		{
			from.linksIndex = (int) mLinks.size();
			mLinks.push_back(NodeLinks());
		}
		
		NodeLinks& nodeLinks = mLinks[from.linksIndex];

		nodeLinks.push_back(to.nodeIndex);
	}
};

class Terrain
{
public:

	typedef NodeGraph<Waypoint> WaypointGraph;

	b2AABB mLimits;
	WaypointGraph mWaypointGraph;
	
	void Init(const b2AABB& limits, float inWaypointRadius, float inWaypointGridSpacing)
	{
		mLimits = limits;

		float waypoint_countf_x = (mLimits.upperBound.x - mLimits.lowerBound.x) / inWaypointGridSpacing;
		float waypoint_countf_y = (mLimits.upperBound.y - mLimits.lowerBound.y) / inWaypointGridSpacing;

		int waypoint_count_x = (int) waypoint_countf_x;
		int waypoint_count_y = (int) waypoint_countf_y;

		float offset_x = (waypoint_countf_x - (((float) (int) waypoint_count_x) * inWaypointGridSpacing)) * 0.5f;
		float offset_y = (waypoint_countf_y - (((float) (int) waypoint_count_y) * inWaypointGridSpacing)) * 0.5f;

		for (int y = 0; y < waypoint_count_y; ++y)
		{
			for (int x = 0; x < waypoint_count_x; ++x)
			{
				WaypointGraph::Node& node = mWaypointGraph.AddNode();
				node.radius = inWaypointRadius;
				node.pos.x = offset_x + (float) x * inWaypointGridSpacing;
				node.pos.y = offset_y + (float) y * inWaypointGridSpacing;
			}
		}

		for (int y = 0; y < waypoint_count_y; ++y)
		{
			for (int x = 0; x < waypoint_count_x; ++x)
			{
				WaypointGraph::Node& node = mWaypointGraph.mNodes[y*waypoint_count_x + x];

				for (int ny = y-1; ny <= y+1; ++ny)
				{
					if (ny >= 0 && ny < waypoint_count_y)
					{
						for (int nx = x-1; nx < x+1; ++nx)
						{
							if (nx >= 0 && nx < waypoint_count_x)
							{
								WaypointGraph::Node& neighbor = mWaypointGraph.mNodes[ny*waypoint_count_x + nx];

								mWaypointGraph.LinkNode(node, neighbor);
							}
						}
					}
				}
			}
		}
	}

	void DrawWaypoints(World& world, const Color& inColor)
	{
		Renderer& renderer = world.GetRenderer();

		for (size_t i = 0; i < mWaypointGraph.mNodes.size(); ++i)
		{
			Waypoint& wpt = mWaypointGraph.mNodes[i];

			renderer.DrawCircle(world.WorldToScreen(wpt.pos), world.WorldToScreen(wpt.radius), inColor, -1.0f, true);
		}
	}

};

#endif