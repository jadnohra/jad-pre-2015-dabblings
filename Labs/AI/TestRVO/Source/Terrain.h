#ifndef TERRAIN_H
#define TERRAIN_H

#include <vector>
#include "Math.h"
#include "World.h"

struct Waypoint
{
	Vector2D pos;
	float radius;

	bool IsStillInside(const Circle& circle) const { return false; }
	bool IsInside(const Circle& circle) const { return false; }

	bool IsStillInsideLink(const Circle& circle, const Waypoint& neighbor) const { return false; }
	bool IsInsideLink(const Circle& circle, const Waypoint& neighbor) const { return false; }
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

	typedef std::vector<int> LinkIndices;

	struct NodeLinks
	{
		int nodeIndex;
		LinkIndices links;
	};
	
	typedef std::vector<NodeLinks> Links;
	typedef std::vector<Node> Nodes;

	Nodes mNodes;
	Links mLinks;

	int AddNode()
	{
		int index = (int) mNodes.size();
		mNodes.push_back(Node());
		mNodes.back().nodeIndex = index;
		mNodes.back().linksIndex = -1;
		return index;
	}

	void LinkNode(int from, int to)
	{
		LinkNode(mNodes[from], mNodes[to]);
	}

	void LinkNode(Node& from, Node& to)
	{
		if (from.linksIndex == -1)
		{
			from.linksIndex = (int) mLinks.size();
			mLinks.push_back(NodeLinks());
			mLinks.back().nodeIndex = from.nodeIndex;
		}
		
		NodeLinks& nodeLinks = mLinks[from.linksIndex];

		nodeLinks.links.push_back(to.nodeIndex);
	}
};

class TerrainAgent
{
public:

	virtual Circle GetTerrainShape() = 0;
};

class Terrain
{
public:

	struct LinkAddress
	{
		int linksIndex;
		int indexInLinks;
	};

	struct AgentLocation
	{
		int waypoints[2];
		LinkAddress links[2];
	};

	struct AgentInfo
	{
		TerrainAgent* agent;
		AgentLocation location;
	};

	typedef NodeGraph<Waypoint> WaypointGraph;
	typedef std::vector<AgentInfo> AgentInfos;
	

	b2AABB mLimits;
	WaypointGraph mWaypointGraph;
	bool mUseTangentsForLinks;
	AgentInfos mAgentInfos;

	Terrain()
	{
	}
	
	void Init(const b2AABB& limits, bool inUseTangentsForLinks)
	{
		mLimits = limits;
		mUseTangentsForLinks = inUseTangentsForLinks;
	}

	int AddAgent(TerrainAgent* pAgent)
	{
		int agentIndex = (int) mAgentInfos.size();
		mAgentInfos.push_back(AgentInfo());
		mAgentInfos.back().agent = pAgent;
		return agentIndex;
	}

	virtual void Update(float time, float dt) 
	{
		for (size_t i=0; i < mAgentInfos.size(); ++i)
		{
			AgentInfo& agentInfo = mAgentInfos[i];

			Circle shape = agentInfo.agent->GetTerrainShape();
	
			int kept_waypoint_count = 0;
			int old_waypoint_count = 0;
			int kept_waypoints[2];
			
			for (int i = 0; i < 2; ++i)
			{
				if (agentInfo.location.waypoints[i] != -1)
				{
					++old_waypoint_count;

					WaypointGraph::Node& node = mWaypointGraph.mNodes[agentInfo.location.waypoints[i]];

					if (node.IsStillInside(shape))
					{
						kept_waypoints[kept_waypoint_count++] = agentInfo.location.waypoints[i];
					}
				}
			}

			int kept_link_count = 0;
			int old_link_count = 0;
			LinkAddress kept_links[2];
			
			for (int i = 0; i < 2; ++i)
			{
				if (agentInfo.location.links[i].linksIndex != -1)
				{
					++old_link_count;

					const LinkAddress& linkAddress = agentInfo.location.links[i];
					const WaypointGraph::NodeLinks& links = mWaypointGraph.mLinks[linkAddress.linksIndex];
					int neighbor = links.links[linkAddress.indexInLinks];
					const WaypointGraph::Node& node = mWaypointGraph.mNodes[links.nodeIndex];

					if (node.IsStillInsideLink(shape, mWaypointGraph.mNodes[neighbor]))
					{
						kept_links[kept_link_count++] = agentInfo.location.links[i];
					}
				}
			}



			int update_waypoints[2];
			int update_waypoint_count = 0;

			if (kept_waypoint_count != 2)
			{
				if (kept_waypoint_count == 0)
				{
					//CONTINUE HERE
					if (old_link_count == 0)
					{
						// Search!
						for (size_t i = 0; i < mWaypointGraph.mNodes.size(); ++i)
						{
							const WaypointGraph::Node& wpt_node = mWaypointGraph.mNodes[i];
						}
					}
					else
					{
						// search nodes at links
					}
				}

				if (kept_waypoint_count == 1)
				{
					update_waypoints[update_waypoint_count++] = kept_waypoints[0];

					WaypointGraph::Node& node = mWaypointGraph.mNodes[kept_waypoints[0]];
					const WaypointGraph::NodeLinks& links = mWaypointGraph.mLinks[node.linksIndex];

					for (size_t i=0; i<links.links.size(); ++i)
					{
						const WaypointGraph::Node& neighbor_wpt_node = mWaypointGraph.mNodes[links.links[i]];

						if (neighbor_wpt_node.IsInside(shape))
						{
							update_waypoints[update_waypoint_count++] = neighbor_wpt_node.nodeIndex;
						}
					}
				}
			}
		}
	}

	void StartBuild()
	{
	}

	void EndBuild()
	{
	}

	void AutoBuild(float inWaypointRadius, float inWaypointGridSpacing)
	{
		StartBuild();

		float waypoint_countf_x = (mLimits.upperBound.x - mLimits.lowerBound.x) / inWaypointGridSpacing;
		float waypoint_countf_y = (mLimits.upperBound.y - mLimits.lowerBound.y) / inWaypointGridSpacing;

		int waypoint_count_x = (int) waypoint_countf_x;
		int waypoint_count_y = (int) waypoint_countf_y;

		float offset_x = (waypoint_countf_x - (((float) (int) waypoint_count_x) * inWaypointGridSpacing)) * 0.5f;
		float offset_y = (waypoint_countf_y - (((float) (int) waypoint_count_y) * inWaypointGridSpacing)) * 0.5f;

		size_t start_index = mWaypointGraph.mNodes.size();

		for (int y = 0; y < waypoint_count_y; ++y)
		{
			for (int x = 0; x < waypoint_count_x; ++x)
			{
				int node_index = mWaypointGraph.AddNode();
				WaypointGraph::Node& node = mWaypointGraph.mNodes[node_index];
				node.radius = inWaypointRadius;
				node.pos.x = offset_x + (float) x * inWaypointGridSpacing;
				node.pos.y = offset_y + (float) y * inWaypointGridSpacing;
			}
		}

		for (int y = 0; y < waypoint_count_y; ++y)
		{
			for (int x = 0; x < waypoint_count_x; ++x)
			{
				WaypointGraph::Node& node = mWaypointGraph.mNodes[start_index + (y*waypoint_count_x + x)];

				for (int ny = y-1; ny <= y+1; ++ny)
				{
					if (ny >= 0 && ny < waypoint_count_y)
					{
						for (int nx = x-1; nx < x+1; ++nx)
						{
							if (nx >= 0 && nx < waypoint_count_x)
							{
								WaypointGraph::Node& neighbor = mWaypointGraph.mNodes[start_index + (ny*waypoint_count_x + nx)];

								mWaypointGraph.LinkNode(node, neighbor);
							}
						}
					}
				}
			}
		}

		EndBuild();
	}

	void CreateLinkQuad(const Waypoint& inWpt1, const Waypoint& inWpt2, Vector2D* outQuad)
	{
		if (mUseTangentsForLinks)
		{
		}
		else
		{
			Vector2D diff = inWpt2.pos - inWpt1.pos;
			Vector2D diff_normal = rotate(diff.Normalized(), 0.5f * MATH_PIf);

			int point_index = 0;

			outQuad[point_index++] = inWpt1.pos + diff_normal * (-inWpt1.radius);
			outQuad[point_index++] = inWpt1.pos + diff_normal * (inWpt1.radius);
			outQuad[point_index++] = inWpt2.pos + diff_normal * (inWpt2.radius);
			outQuad[point_index++] = inWpt2.pos + diff_normal * (-inWpt2.radius);
		}
	}

	void DrawLink(World& inWorld, Renderer& inRenderer, Vector2D* inQuad, const Color& inColor)
	{
		inRenderer.DrawLine(inWorld.WorldToScreen(inQuad[1]), inWorld.WorldToScreen(inQuad[2]), inColor);
		inRenderer.DrawLine(inWorld.WorldToScreen(inQuad[3]), inWorld.WorldToScreen(inQuad[0]), inColor);
	}


	void DrawWaypoints(World& inWorld, bool inDrawLinks, const Color& inColor, const Color& inLinkColor)
	{
		Renderer& renderer = inWorld.GetRenderer();

		if (inDrawLinks)
		{
			for (size_t i = 0; i < mWaypointGraph.mNodes.size(); ++i)
			{
				const WaypointGraph::Node& wpt_node = mWaypointGraph.mNodes[i];
				const WaypointGraph::NodeLinks& links = mWaypointGraph.mLinks[wpt_node.linksIndex];

				for (size_t i=0; i<links.links.size(); ++i)
				{
					const WaypointGraph::Node& neighbor_wpt_node = mWaypointGraph.mNodes[links.links[i]];

					Vector2D quad[4];

					CreateLinkQuad(wpt_node, neighbor_wpt_node, quad);
					DrawLink(inWorld, renderer, quad, inLinkColor);
				}
			}
		}

		for (size_t i = 0; i < mWaypointGraph.mNodes.size(); ++i)
		{
			const WaypointGraph::Node& wpt_node = mWaypointGraph.mNodes[i];

			renderer.DrawCircle(inWorld.WorldToScreen(wpt_node.pos), inWorld.WorldToScreen(wpt_node.radius), inColor, -1.0f, true);
		}
	}

};

#endif