#ifndef TERRAIN_H
#define TERRAIN_H

#include <vector>
#include "Math.h"
#include "World.h"


// TODO 3 links at the same time!!!! 
// 1 left 1 down 1 diagonal!
// maybe only 1 wpt at a time + restrict waypoint gen?


struct Waypoint
{
	Vector2D pos;
	float radius;
	float origRadius;

	static int CreateLinkQuad(const Waypoint& inWpt1, const Waypoint& inWpt2, bool inUseTangentsForLinks, Vector2D* outQuad)
	{
		int unique_point_count = 4;

		if (inUseTangentsForLinks)
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

			if (inWpt1.radius == 0.0f)
				--unique_point_count;

			if (inWpt2.radius == 0.0f)
				--unique_point_count;
		}

		return unique_point_count;
	}

	bool IsStillInside(const Circle& circle) const 
	{ 
		return IsInside(circle); 
	}
	
	bool IsInside(const Circle& circle) const 
	{ 
		return Distance(pos, circle.pos) <= radius - circle.radius;
	}

	bool IsStillInsideLink(const Circle& circle, const Waypoint& neighbor, bool inUseTangentsForLinks) const 
	{ 
		return IsInsideLink(circle, neighbor, inUseTangentsForLinks); 
	}
	
	bool IsInsideLink(const Circle& circle, const Waypoint& neighbor, bool inUseTangentsForLinks) const 
	{ 
		Vector2D quad[4];

		int unique_point_count = CreateLinkQuad(*this, neighbor, inUseTangentsForLinks, quad);

		if (unique_point_count <= 2)
			return false;

		Vector2D poly_center = Vector2D::kZero;

		for (int i=0; i<4; ++i)
		{
			poly_center = poly_center + (quad[i] * 0.25f);
		}

		for (int i=0; i<4; ++i)
		{
			const Vector2D& pt1 = quad[i];
			Vector2D pt2 = quad[(i+1) % 4];
			
			Vector2D dir = pt2 - pt1;

			if (dir != Vector2D::kZero)
			{
				float dot1 = GetPointSideOfLine(pt1, dir, circle.pos);
				float dot2 = GetPointSideOfLine(pt1, dir, poly_center); 

				if (dot1 < 0.0f && dot2 > 0.0f)
					return false;
				if (dot1 > 0.0f && dot2 < 0.0f)
					return false;

				float dist = sqrtf(DistancePointLineSquared(pt1, dir, circle.pos));

				if (dist < circle.radius)
					return false;
			}
		}

		return true; 
	}

	float GetAABBlockedRadiusForLink(const b2AABB& box, const Waypoint& neighbor, float inRadius, float inNeighborRadius, bool inUseTangentsForLinks) const 
	{ 
		// This function is hacked together and not really correct. needs fixing

		Vector2D lineDir = neighbor.pos - pos;
		
		Vector2D quad[4];
		CreateBoxQuad(box, quad);

		float min_radius_sq = FLT_MAX;

		bool curr_side_set = false;
		float curr_side = 0.0f;

		for (int i=0; i<4; ++i)
		{
			float u;
			float dist_sq = DistancePointLineSquared(pos, lineDir, quad[i], &u);

			if (u >= 0.0f && u <= 1.0f)
			{
				if (dist_sq < min_radius_sq)
					min_radius_sq = dist_sq;

				float point_side = GetPointSideOfLine(pos, lineDir, quad[i]);

				if (point_side == 0.0f) //line center touches box 
					return 0.0f;

				if (curr_side_set == false)
				{
					curr_side_set = true;
					curr_side = point_side;
				}
				else
				{
					//line center inside box 

					if ((point_side > 0.0 && curr_side < 0.0f)
						|| (point_side < 0.0 && curr_side > 0.0f))
						return 0.0f;
				}
			}
		}

		if (min_radius_sq < inRadius * inRadius 
			|| min_radius_sq < inNeighborRadius * inNeighborRadius)
		{
			float min_radius  = sqrtf(min_radius_sq);
			min_radius = std::min(min_radius, inRadius);
			min_radius = std::min(min_radius, inNeighborRadius);

			return min_radius;
		}

		return -1.0f;
	}

	float GetAABBlockedRadius(const b2AABB& box, float inRadius)
	{ 
		Vector2D quad[4];

		int unique_point_count = CreateBoxQuad(box, quad);

		if (unique_point_count >= 3)
		{
			bool is_inside = true;

			Vector2D poly_center = Vector2D::kZero;

			for (int i=0; i<4; ++i)
			{
				poly_center = poly_center + (quad[i] * 0.25f);
			}

			for (int i=0; i<4; ++i)
			{
				const Vector2D& pt1 = quad[i];
				const Vector2D& pt2 = quad[(i+1) % 4];

				Vector2D dir = pt2 - pt1;

				if (dir != Vector2D::kZero)
				{
					float dot1 = GetPointSideOfLine(pt1, dir, pos);
					float dot2 = GetPointSideOfLine(pt1, dir, poly_center); 

					if ((dot1 < 0.0f && dot2 > 0.0f) || (dot1 > 0.0f && dot2 < 0.0f))
					{
						is_inside = false;
						break;
					}
				}
			}

			if (is_inside)
				return 0.0f;
		}

		float ret = inRadius;
		float dist;

		for (int i=0; i<4; ++i)
		{
			const Vector2D& pt1 = quad[i];
			const Vector2D& pt2 = quad[(i+1) % 4];
			Vector2D dir = pt2 - pt1;

			float u;
			dist = sqrtf(DistancePointLineSquared(pt1, dir, pos, &u));

			if (u >= 0.0f && u <= 1.0f && dist < ret)
				ret = dist;

			dist = Distance(pos, quad[i]);
			if (dist < ret)
				ret = dist;
		}

		return ret; 
	}
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
		int linkFrom;
		int linkTo;

		LinkAddress(int inLinksIndex = -1, int inIndexInLinks = -1, int inLinkFrom = -1, int inLinkTo = -1)
		:	linksIndex(inLinksIndex)
		,	indexInLinks(inIndexInLinks)
		,	linkFrom(inLinkFrom)
		,	linkTo(inLinkTo)
		{
			if (linkFrom > linkTo)
			{
				int temp = linkTo;
				linkTo = linkFrom;
				linkFrom = temp;
			}
		}

		inline bool operator==(const LinkAddress& address) const
		{
			return linkFrom == address.linkFrom
					&& linkTo == address.linkTo;
		}
	};

	struct AgentLocation
	{
		int waypoints[2];
		LinkAddress links[2];

		AgentLocation()
		{
			waypoints[0] = -1;
			waypoints[1] = -1;
		}
	};

	struct AgentInfo
	{
		TerrainAgent* agent;
		AgentLocation location;
	};

	struct ObstacleInfo
	{
		b2AABB box;
	};

	typedef NodeGraph<Waypoint> WaypointGraph;
	typedef std::vector<AgentInfo> AgentInfos;
	typedef std::vector<ObstacleInfo> ObstacleInfos;
	

	b2AABB mLimits;
	WaypointGraph mWaypointGraph;
	bool mUseTangentsForLinks;
	AgentInfos mAgentInfos;
	ObstacleInfos mObstacleInfos;

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

	int AddStaticObstacle(const b2AABB& inBox, bool inDirectUpdate = false)
	{
		int index = (int) mObstacleInfos.size();
		mObstacleInfos.push_back(ObstacleInfo());
		mObstacleInfos.back().box = inBox;

		if (inDirectUpdate)
			UpdateStaticObstacle(mObstacleInfos.back());

		return index;
	}

	void UpdateStaticObstacles()
	{
		for (size_t i=0; i < mObstacleInfos.size(); ++i)
		{
			UpdateStaticObstacle(mObstacleInfos[i]);
		}
	}

	void UpdateStaticObstacle(const ObstacleInfo& obstacleInfo)
	{
		for (size_t i = 0; i < mWaypointGraph.mNodes.size(); ++i)
		{
			WaypointGraph::Node& wpt_node = mWaypointGraph.mNodes[i];

			float new_radius = wpt_node.GetAABBlockedRadius(obstacleInfo.box, wpt_node.origRadius);

			if (new_radius < wpt_node.radius)
				wpt_node.radius = new_radius;

			if (wpt_node.linksIndex >= 0)
			{
				const WaypointGraph::NodeLinks& links = mWaypointGraph.mLinks[wpt_node.linksIndex];

				for (size_t i=0; i<links.links.size(); ++i)
				{
					int neighbor_index = links.links[i];
					const WaypointGraph::Node& neighbor_wpt_node = mWaypointGraph.mNodes[neighbor_index];

					float test_radius = wpt_node.GetAABBlockedRadiusForLink(obstacleInfo.box, neighbor_wpt_node, wpt_node.radius, neighbor_wpt_node.origRadius, mUseTangentsForLinks);

					if (test_radius >= 0.0f && (test_radius < wpt_node.radius))
					{
						wpt_node.radius = test_radius;
					}
				}
			}
		}
	}

	template<typename T>
	static bool IsIn(T inTestVal, T* inCandidates, int inCandidateCount)
	{
		for (int i = 0; i < inCandidateCount; ++i)
		{
			if (inTestVal == inCandidates[i])
				return true;
		}
		return false;
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

					if (node.IsStillInsideLink(shape, mWaypointGraph.mNodes[neighbor], mUseTangentsForLinks))
					{
						kept_links[kept_link_count++] = agentInfo.location.links[i];
					}
				}
			}

			int update_waypoints[2];
			int update_waypoint_count = 0;

			for (int i = 0; i < kept_waypoint_count; ++i)
			{
				update_waypoints[update_waypoint_count++] = kept_waypoints[i];
			}

			if (update_waypoint_count != 2)
			{
				// we have space left for waypoints.
				
				if (old_link_count != 0)
				{
					// we had some links, try their waypoints.
					for (int i = 0; i < 2; ++i)
					{
						if (agentInfo.location.links[i].linksIndex != -1)
						{
							const LinkAddress& linkAddress = agentInfo.location.links[i];
							const WaypointGraph::NodeLinks& links = mWaypointGraph.mLinks[linkAddress.linksIndex];
							int node = links.nodeIndex;
							int neighbor = links.links[linkAddress.indexInLinks];
							
							if (!IsIn(node, update_waypoints, update_waypoint_count)
								&& mWaypointGraph.mNodes[node].IsInside(shape))
							{
								update_waypoints[update_waypoint_count++] = node;
							}
							else if (!IsIn(neighbor, update_waypoints, update_waypoint_count)
									 && mWaypointGraph.mNodes[neighbor].IsInside(shape))
							{
								update_waypoints[update_waypoint_count++] = neighbor;
							}
						}

						if (update_waypoint_count == 2)
							break;
					}
				}

				if (update_waypoint_count != 2)
				{
					if (update_waypoint_count == 0)
					{
						// Full search, we have no spatial search, could be useful
						for (size_t i = 0; i < mWaypointGraph.mNodes.size(); ++i)
						{
							const WaypointGraph::Node& wpt_node = mWaypointGraph.mNodes[i];

							if (wpt_node.IsInside(shape))
							{
								update_waypoints[update_waypoint_count++] = wpt_node.nodeIndex;
								break;
							}
						}
					}

					if (update_waypoint_count == 1)
					{
						// Search neighbors, we have no spatial search, could be useful (for nodes that are close but not neighbors)
						WaypointGraph::Node& node = mWaypointGraph.mNodes[update_waypoints[0]];

						if (node.linksIndex >= 0)
						{
							const WaypointGraph::NodeLinks& links = mWaypointGraph.mLinks[node.linksIndex];

							for (size_t i=0; i<links.links.size(); ++i)
							{
								int neighbor_index = links.links[i];
								const WaypointGraph::Node& neighbor_wpt_node = mWaypointGraph.mNodes[neighbor_index];

								if (neighbor_wpt_node.IsInside(shape))
								{
									update_waypoints[update_waypoint_count++] = neighbor_index;
									break;
								}
							}
						}
					}
				}
			}

			LinkAddress update_links[2];
			int update_link_count = 0;

			for (int i = 0; i < kept_link_count; ++i)
			{
				update_links[update_link_count++] = kept_links[i];
			}
	
			if (update_link_count != 2)
			{
				// we have space left for links.

				if (old_waypoint_count != 0)
				{
					// we had some waypoints, try their links.
					for (int i = 0; i < 2; ++i)
					{
						if (agentInfo.location.waypoints[i] != -1)
						{
							WaypointGraph::Node& node = mWaypointGraph.mNodes[agentInfo.location.waypoints[i]];

							if (node.linksIndex >= 0)
							{
								const WaypointGraph::NodeLinks& links = mWaypointGraph.mLinks[node.linksIndex];

								for (size_t j=0; j<links.links.size(); ++j)
								{
									int neighbor_index = links.links[j];
									const WaypointGraph::Node& neighbor_wpt_node = mWaypointGraph.mNodes[neighbor_index];

									LinkAddress testLinkAddress(node.linksIndex, (int)j, node.nodeIndex, neighbor_index);

									if (!IsIn(testLinkAddress, update_links, update_link_count)
										&& node.IsInsideLink(shape, neighbor_wpt_node, mUseTangentsForLinks))
									{
										update_links[update_link_count++] = testLinkAddress;
										break;
									}
								}

							}
						}

						if (update_link_count == 2)
							break;
					}
				}

				if (update_link_count != 2)
				{

					if (update_link_count == 0)
					{
						for (size_t i=0; i<mWaypointGraph.mLinks.size(); ++i)
						{
							const WaypointGraph::NodeLinks& links = mWaypointGraph.mLinks[i];

							for (size_t j=0; j<links.links.size(); ++j)
							{
								int neighbor_index = links.links[j];
								const WaypointGraph::Node& wpt_node = mWaypointGraph.mNodes[links.nodeIndex];
								const WaypointGraph::Node& neighbor_wpt_node = mWaypointGraph.mNodes[neighbor_index];

								if (wpt_node.IsInsideLink(shape, neighbor_wpt_node, mUseTangentsForLinks))
								{
									update_links[update_link_count++] = LinkAddress((int) i, (int) j, wpt_node.nodeIndex, neighbor_index);;
									break;
								}
							}

							if (update_link_count == 1)
								break;
						}

						//this code and all searching code can be improved with modular functions
						// and a code on top that detects if a function found anything new then using 
						// others recursively
						//e.g we might find some new waypoints connected to this link...
					}

					if (update_link_count == 1)
					{
						// search links from our link's waypoints
						const WaypointGraph::NodeLinks& node_links = mWaypointGraph.mLinks[update_links[0].linksIndex];
						int relevant_nodes[] = { node_links.nodeIndex, node_links.links[update_links[0].indexInLinks] };

						for (int i = 0; i < 2; ++i)
						{
							WaypointGraph::Node& node = mWaypointGraph.mNodes[relevant_nodes[i]];

							if (node.linksIndex >= 0)
							{
								const WaypointGraph::NodeLinks& links = mWaypointGraph.mLinks[node.linksIndex];

								for (size_t j=0; j<links.links.size(); ++j)
								{
									int neighbor_index = links.links[j];
									const WaypointGraph::Node& neighbor_wpt_node = mWaypointGraph.mNodes[neighbor_index];

									LinkAddress testLinkAddress(node.linksIndex, (int) j, node.nodeIndex, neighbor_index);

									if (!IsIn(testLinkAddress, update_links, update_link_count)
										&& node.IsInsideLink(shape, neighbor_wpt_node, mUseTangentsForLinks))
									{
										update_links[update_link_count++] = testLinkAddress;
										break;
									}
								}

								if (update_link_count == 2)
									break;
							}
						}
					}
					else
					{
						
					}
				}
			}

			for (int i = 0; i < 2; ++i)
			{
				agentInfo.location.waypoints[i] = -1;
				agentInfo.location.links[i].linksIndex = -1;
			}

			for (int i = 0; i < update_waypoint_count; ++i)
			{
				agentInfo.location.waypoints[i] = update_waypoints[i];
			}

			for (int i = 0; i < update_link_count; ++i)
			{
				agentInfo.location.links[i] = update_links[i];
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
				node.origRadius = inWaypointRadius;
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
						for (int nx = x-1; nx <= x+1; ++nx)
						{
							if (nx >= 0 && nx < waypoint_count_x)
							{
								if (ny != y || nx != x)
								{
									WaypointGraph::Node& neighbor = mWaypointGraph.mNodes[start_index + (ny*waypoint_count_x + nx)];

									mWaypointGraph.LinkNode(node, neighbor);
								}
							}
						}
					}
				}
			}
		}

		EndBuild();
	}

	void DrawLink(World& inWorld, Renderer& inRenderer, Vector2D* inQuad, const Color& inColor, float inLineWidth = 0.7f)
	{
		inRenderer.DrawLine(inWorld.WorldToScreen(inQuad[1]), inWorld.WorldToScreen(inQuad[2]), inColor, -1.0f, inLineWidth);
		inRenderer.DrawLine(inWorld.WorldToScreen(inQuad[3]), inWorld.WorldToScreen(inQuad[0]), inColor, -1.0f, inLineWidth);
	}


	void DrawWaypoints(World& inWorld, bool inDrawLinks, const Color& inColor, const Color& inLinkColor)
	{
		Renderer& renderer = inWorld.GetRenderer();

		if (inDrawLinks)
		{
			for (size_t i = 0; i < mWaypointGraph.mNodes.size(); ++i)
			{
				const WaypointGraph::Node& wpt_node = mWaypointGraph.mNodes[i];

				if (wpt_node.linksIndex >= 0)
				{
					const WaypointGraph::NodeLinks& links = mWaypointGraph.mLinks[wpt_node.linksIndex];

					for (size_t i=0; i<links.links.size(); ++i)
					{
						const WaypointGraph::Node& neighbor_wpt_node = mWaypointGraph.mNodes[links.links[i]];

						Vector2D quad[4];

						int quad_point_count = Waypoint::CreateLinkQuad(wpt_node, neighbor_wpt_node, mUseTangentsForLinks, quad);

						if (quad_point_count >= 3)
							DrawLink(inWorld, renderer, quad, inLinkColor, 0.3f);
					}
				}
			}
		}

		for (size_t i = 0; i < mWaypointGraph.mNodes.size(); ++i)
		{
			const WaypointGraph::Node& wpt_node = mWaypointGraph.mNodes[i];

			renderer.DrawCircle(inWorld.WorldToScreen(wpt_node.pos), inWorld.WorldToScreen(wpt_node.radius), inColor, -1.0f, true);
		}
	}


	void DrawTerrainInfo(World& inWorld, const TerrainAgent* pAgent, const Color& inColor, const Color& inLinkColor)
	{
		for (size_t i=0; i < mAgentInfos.size(); ++i)
		{
			AgentInfo& agentInfo = mAgentInfos[i];

			if (agentInfo.agent == pAgent)
			{
				DrawTerrainInfo(inWorld, agentInfo, inColor, inLinkColor, 3.0f);
				break;
			}
		}
	}

	void DrawObstacles(World& inWorld, const Color& inColor, float inLineWidth = 0.7f)
	{
		Renderer& renderer = inWorld.GetRenderer();
		Vector2D quad[4];

		for (size_t i=0; i < mObstacleInfos.size(); ++i)
		{
			ObstacleInfo& obstacleInfo = mObstacleInfos[i];

			CreateBoxQuad(obstacleInfo.box, quad);
			renderer.DrawQuad(inWorld.WorldToScreen(quad[0]), 
							  inWorld.WorldToScreen(quad[1]), 
							  inWorld.WorldToScreen(quad[2]), 
							  inWorld.WorldToScreen(quad[3]), inColor, -1.0f, inLineWidth, true);
		}
	}

	void DrawTerrainInfo(World& inWorld, const AgentInfo& agentInfo, const Color& inColor, const Color& inLinkColor, float inLineWidth = 0.7f)
	{
		Renderer& renderer = inWorld.GetRenderer();

		for (int i = 0; i < 2; ++i)
		{
			if (agentInfo.location.links[i].linksIndex != -1)
			{
				const WaypointGraph::NodeLinks& links = mWaypointGraph.mLinks[agentInfo.location.links[i].linksIndex];
				int neighbor = links.links[agentInfo.location.links[i].indexInLinks];

				Vector2D quad[4];

				int quad_point_count = Waypoint::CreateLinkQuad(mWaypointGraph.mNodes[links.nodeIndex], mWaypointGraph.mNodes[neighbor], mUseTangentsForLinks, quad);
				DrawLink(inWorld, renderer, quad, inLinkColor, inLineWidth);
			}
		}

		for (int i = 0; i < 2; ++i)
		{
			if (agentInfo.location.waypoints[i] != -1)
			{
				const WaypointGraph::Node& wpt_node = mWaypointGraph.mNodes[agentInfo.location.waypoints[i]];
				renderer.DrawCircle(inWorld.WorldToScreen(wpt_node.pos), inWorld.WorldToScreen(wpt_node.radius), inColor, -1.0f);
			}
		}
	}
};

#endif