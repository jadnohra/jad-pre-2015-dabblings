#ifndef TERRAIN_H
#define TERRAIN_H

#include <vector>
#include "Math.h"
#include "World.h"
#include "AStar.h"


// TODO 3 links at the same time!!!! 
// 1 left 1 down 1 diagonal!
// maybe only 1 wpt at a time + restrict waypoint gen?


struct WaypointLink
{
	float minRadius;

	WaypointLink()
		: minRadius(FLT_MAX)
	{}

};

struct Waypoint
{
	Vector2D pos;
	float radius;
	float origRadius;

	static int CreateLinkQuad(const Waypoint& inWpt1, const Waypoint& inWpt2, const WaypointLink* pWptLink, bool inUseTangentsForLinks, float inShrinkRadius, Vector2D* outQuad)
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

			if (pWptLink && pWptLink->minRadius != FLT_MAX)
			{
				float radius = std::max(0.0f, (pWptLink->minRadius - inShrinkRadius));

				outQuad[point_index++] = inWpt1.pos + diff_normal * (-radius);
				outQuad[point_index++] = inWpt1.pos + diff_normal * (radius);
				outQuad[point_index++] = inWpt2.pos + diff_normal * (radius);
				outQuad[point_index++] = inWpt2.pos + diff_normal * (-radius);

				if (radius <= 0.0f)
					unique_point_count = 1;
			}
			else
			{
				float radius1 = std::max(0.0f, (inWpt1.radius - inShrinkRadius));
				float radius2 = std::max(0.0f, (inWpt2.radius - inShrinkRadius));

				outQuad[point_index++] = inWpt1.pos + diff_normal * (-radius1);
				outQuad[point_index++] = inWpt1.pos + diff_normal * (radius1);
				outQuad[point_index++] = inWpt2.pos + diff_normal * (radius2);
				outQuad[point_index++] = inWpt2.pos + diff_normal * (-radius2);

				if (radius1 == 0.0f)
					--unique_point_count;

				if (radius2 == 0.0f)
					--unique_point_count;
			}
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

	bool IsInside(const Vector2D& pt) const 
	{ 
		return Distance(pt, pos) <= radius;
	}

	bool IsStillInsideLink(const Circle& circle, const Waypoint& neighbor, const WaypointLink* pWptLink, bool inUseTangentsForLinks) const 
	{ 
		return IsInsideLink(circle, neighbor, pWptLink, inUseTangentsForLinks); 
	}
	
	bool IsInsideLink(const Circle& circle, const Waypoint& neighbor, const WaypointLink* pWptLink, bool inUseTangentsForLinks) const 
	{ 
		Vector2D quad[4];

		int unique_point_count = CreateLinkQuad(*this, neighbor, pWptLink, inUseTangentsForLinks, 0.0f, quad);

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

	bool IsInsideLink(const Vector2D& pt, const Waypoint& neighbor, const WaypointLink* pWptLink, bool inUseTangentsForLinks) const 
	{ 
		Vector2D quad[4];

		int unique_point_count = CreateLinkQuad(*this, neighbor, pWptLink, inUseTangentsForLinks, 0.0f, quad);

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
				float dot1 = GetPointSideOfLine(pt1, dir, pt);
				float dot2 = GetPointSideOfLine(pt1, dir, poly_center); 

				if (dot1 < 0.0f && dot2 > 0.0f)
					return false;
				if (dot1 > 0.0f && dot2 < 0.0f)
					return false;

				float dist = sqrtf(DistancePointLineSquared(pt1, dir, pt));

				if (dist < 0.0f)
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
			if (quad[i] != quad[(i+1)%4])
			{
				Vector2D inters;

				if (IntersectSegments(quad[i],quad[(i+1)%4], pos, neighbor.pos, inters))
				{
					return 0.0f;
				}
			}

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

	bool operator==(const Waypoint& comp) const
	{
		return pos == comp.pos && radius == comp.radius && origRadius == comp.origRadius;
	}
};


template <typename NodeT, typename EdgeT>
class NodeGraph
{
public:

	struct Node : NodeT
	{
		int linksIndex;
		int nodeIndex;

		bool operator==(const Node& comp) const
		{
			return nodeIndex == comp.nodeIndex;
		}
	};

	struct Edge : EdgeT
	{
		int targetNodeIndex;

		explicit Edge(int targetIndex) : targetNodeIndex(targetIndex)
		{
		}

		inline operator int() const
		{
			return targetNodeIndex;
		}
	};

	typedef std::vector<Edge> Edges;

	struct NodeLinks
	{
		int nodeIndex;
		Edges nodeEdges;
	};
	
	typedef std::vector<NodeLinks> Links;
	typedef std::vector<Node> Nodes;
	typedef int Index;
	typedef Edges EdgeIndices;

	enum { InvalidIndex = -1 };

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

		nodeLinks.nodeEdges.push_back(Edge(to.nodeIndex));
	}

	const Node& getNode(int index) const { return mNodes[index]; } 
	//const Edge& getEdge(const int& index) const { return index; } 
	const EdgeIndices& getEdgeIndices(int nodeIndex) const { return mLinks[getNode(nodeIndex).linksIndex].nodeEdges; }
};


template<typename Graph>
class NodeGraphView
{
public:

	// ----------------------------------------------------------------------------
	// Type aliases.
	// ----------------------------------------------------------------------------

	typedef typename Graph::Node Node;
	typedef typename Graph::Edge Edge;
	typedef typename Graph::Index Index;
	typedef typename Graph::EdgeIndices EdgeIndices;

public:

	// ----------------------------------------------------------------------------
	// Concept types.
	// ----------------------------------------------------------------------------

	struct EdgeIterator;
	typedef void* LiveNodeIterator;

public:

	// ----------------------------------------------------------------------------
	// Concept functions.
	// ----------------------------------------------------------------------------

	bool isCompatible(Graph& graph) { return true; }
	const Node& node(Graph& graph, const Index& nodeIndex) const { return graph.getNode(nodeIndex); }
	const Edge& edge(Graph& graph, const EdgeIterator& edgeIter) { return *(edgeIter.m_curr); }
	const Index edgeIndex(const EdgeIterator& edgeIter) { return *(edgeIter.m_curr); }
	EdgeIterator edgeIterator(Graph& graph, const Index& nodeIndex) { return EdgeIterator((edgeIndices(graph, nodeIndex))); }
	bool isAtEnd(EdgeIterator& iter) { return iter.m_curr == iter.m_edgeIndices.end(); }
	bool hasEdge(EdgeIterator& iter) { return !isAtEnd(iter); }
	void nextEdge(EdgeIterator& iter) { ++iter.m_curr; }

	LiveNodeIterator liveNodeIterator() { return NULL; }
	const Node* getLiveNode(LiveNodeIterator& iter) { return NULL; }
	Index getLiveNodeIndex(LiveNodeIterator& iter) { return InvalidNodeIndex; }
	void nextLiveNode(LiveNodeIterator& iter) {}

protected:

	const EdgeIndices& edgeIndices(Graph& graph, const Index& nodeIndex) 
	{ 
		return graph.getEdgeIndices(nodeIndex); 
	}

public:

	// ----------------------------------------------------------------------------
	// Concept types implementations.
	// ----------------------------------------------------------------------------

	struct EdgeIterator 
	{
		typedef typename Graph::EdgeIndices EdgeIndices;
		typedef typename EdgeIndices::const_iterator EdgeIndicesIter;

		EdgeIterator(const EdgeIndices& edgeIndices) 
		:	m_edgeIndices(edgeIndices) 
		,	m_curr(m_edgeIndices.begin())
		{
		}

		const EdgeIndices& m_edgeIndices;
		EdgeIndicesIter m_curr;
	};
};


typedef NodeGraph<Waypoint, WaypointLink> WaypointGraph;


class WaypointPathCostEval
{
public:

	/// flag indicating the Heuristic's admissibility
	/// (true might allow a faster search)
	enum { IsAdmissibleHeuristic = true };

	/// flag indicating if the Evalutor can break ties using breakTie
	enum { CanBreakTies = false };

	/// The type used as Path Cost Value
	typedef float CostValue;

public:

	const WaypointGraph& mGraph;
	float mObjectRadius;

	WaypointPathCostEval(const WaypointGraph& graph, float objRadius)
	: mGraph(graph)
	, mObjectRadius(objRadius)
	{

	}

	/**
	 * Gets the Path cost 'g' between 2 Nodes connected by an Edge
	 *
	 * @return				The path cost ('g')
	 */
	template<typename Node, typename Edge, typename EdgeIndex>
	CostValue getEdgeCost(const Node& source, const Edge& edge, EdgeIndex edgeIndex)
	{
		if (source.radius <= mObjectRadius)
			return FLT_MAX;
		if (edge.minRadius <= mObjectRadius)
			return FLT_MAX;
		if (mGraph.getNode(edge).radius <= mObjectRadius)
			return FLT_MAX;

		return Distance(source.pos, mGraph.getNode(edge).pos);
	}
	
	/**
	 * Gets the Heuristic Path cost 'h' between 2 Nodes connected
	 *
	 * @return				The heuristic path cost ('h')
	 */
	template<typename Node>
	CostValue getHeuristicCost(const Node& from, const Node& to)
	{
		return Distance(from.pos, to.pos);
	}

	/**
	 * Breaks a tie between two nodes having the same g+h cost.
	 *
	 * @param stateManager		The node state manager. 
	 * @param refStateIndex      The reference node's state index
	 * @param compStateIndex     The compared node's state index
	 *
	 * @return			true if comp is strictly better than ref
	 */
	template<typename NodeStateManager, typename NodeStateIndex>
	bool breakTie(NodeStateManager& stateManager, 
					NodeStateIndex refStateIndex, NodeStateIndex compStateIndex) 
	{
		if (CanBreakTies)
		{
			//this breaks the tie using h
			return stateManager.h(compStateIndex) < stateManager.h(refStateIndex);
		}
		
		return false;
	}
};

struct PathNode
{
	int wptIndex;
	Vector2D pos;

	PathNode(int wptIndex_, const Vector2D& pos_)
		:	wptIndex(wptIndex_), pos(pos_)
	{

	}

	PathNode(int wptIndex_)
		:	wptIndex(wptIndex_)
	{
	}
};

class Path
{
public:

	typedef astar::AStarState<WaypointGraph, WaypointPathCostEval, NodeGraphView<WaypointGraph>> AStarState;
	typedef std::vector<PathNode> PathNodes;

	PathNodes mPathNodes;
	WaypointGraph* mpWaypoints;

	Path() : mpWaypoints(NULL) {}

	bool Find(WaypointGraph& waypoints, int from, int to, float objRadius)
	{
		AStarState state;
		WaypointPathCostEval pathCostEval(waypoints, objRadius);

		mpWaypoints = &waypoints;
		mPathNodes.clear();

		if (waypoints.mNodes[to].radius < objRadius)
			return false;

		state.init(waypoints, pathCostEval);
		PathSearchInitStatus initStatus = state.initSearch(from, to);
		if (initStatus == PathSearchInitOK)
		{
			PathSearchStatus status;

			do
			{
				status = AStarAlgorithm::step(state);
			}
			while (status == PathSearching);

			if (status == PathSearchFound)
			{
				std::vector<int> tempPath;

				state.extractReversePath(tempPath);
				std::reverse(tempPath.begin(), tempPath.end());

				mPathNodes.reserve(tempPath.size());
				for (size_t i = 0; i < tempPath.size(); ++i)
				{
					mPathNodes.push_back(PathNode(tempPath[i], waypoints.getNode(tempPath[i]).pos));
				}
			}
			
			return status == PathSearchFound;
		}
		else
		{
			return (initStatus == PathSearchInitSameNode);
		}
		
		return false;
	}

	Path& operator=(const Path& ref)
	{
		mpWaypoints = ref.mpWaypoints;
		mPathNodes = ref.mPathNodes;

		return *this;
	}

	int Length() const { return (int) mPathNodes.size(); }
	const PathNode& GetNode(int index) const { return mPathNodes[index]; }
	PathNode& GetNode(int index) { return mPathNodes[index]; }
	Vector2D GetPoint(int index) const { return mPathNodes[index].pos; }
};


class TerrainAgent
{
public:

	TerrainAgent() : mpTerrain(NULL) {}

	virtual Circle GetTerrainShape() = 0;
	virtual const Path* GetPath(int& currToIndex, bool& hasTempAvoidPt, Vector2D& currTempAvoidancePt) { return NULL; }
	Terrain* mpTerrain;
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

	struct NavigableNode
	{
		int waypoint;
		LinkAddress link;

		NavigableNode(int wpt)
		{
			waypoint = wpt;
		}

		NavigableNode(const LinkAddress& link_)
		{
			waypoint = -1;
			link = link_;
		}

		bool Contains(Terrain& terrain, const Vector2D& point, float radius) const
		{
			if (waypoint != -1)
			{
				return terrain.mWaypointGraph.getNode(waypoint).IsInside(Circle(point, radius));
			}
			else
			{
				WaypointLink* pWptLink = terrain.GetWptLink(link);
				return terrain.mWaypointGraph.getNode(link.linkFrom).IsInsideLink(Circle(point, radius), terrain.mWaypointGraph.getNode(link.linkTo), pWptLink, terrain.mUseTangentsForLinks);
			}
		}

		bool IntersectRay(Terrain& terrain, const Vector2D& rayOrig, const Vector2D& rayDir, float pointRadius, Vector2D& outIntersectionPoint) const
		{
			if (waypoint != -1)
			{
				float t, u;
				int inter_count = IntersectLineCircle(rayOrig, rayDir, 
														terrain.mWaypointGraph.getNode(waypoint).pos, 
														std::max(0.0f, terrain.mWaypointGraph.getNode(waypoint).radius - pointRadius), t, u);

				if (inter_count > 0)
				{
					float inter_t = -1.0f;

					if (t > 0.0f)
					{
						inter_t = t;
					}
					else if (u > 0.0f)
					{
						inter_t = u;
					}
					else if (t >= 0.0f)
					{
						inter_t = t;
					}
					else if (u >= 0.0f)
					{
						inter_t = u;
					}

					if (inter_t >= 0.0f)
					{
						outIntersectionPoint = rayOrig + (rayDir * inter_t);
						return true;
					}
				}
			}
			else
			{
				Vector2D quad[4];
				WaypointLink* pWptLink = terrain.GetWptLink(link);
				Waypoint::CreateLinkQuad(terrain.mWaypointGraph.getNode(link.linkFrom),
										 terrain.mWaypointGraph.getNode(link.linkTo),
										 pWptLink,
										 terrain.mUseTangentsForLinks,
										 pointRadius, quad);
				float min_t = FLT_MAX;
				float non_zero_min_t = FLT_MAX;
				
				for (int i = 0; i < 4; ++i)
				{
					float t, u;

					if (IntersectLines(rayOrig, rayOrig + rayDir, quad[i], quad[(i+1)%4], t, u)
						&& (u >= 0.0f && u <= 1.0f))
					{
						if (t >= 0.0f && t <= min_t)
							min_t = t;

						if (t > 0.0f && t <= min_t)
							non_zero_min_t = t;
					}
				}

				if (non_zero_min_t != FLT_MAX)
				{
					outIntersectionPoint = rayOrig + rayDir * non_zero_min_t;
					return true;
				}
				else
				{
					if (min_t != FLT_MAX)
					{
						outIntersectionPoint = rayOrig + rayDir * min_t;
						return true;
					}
				}
			}

			return false;
		}
	};

	typedef std::vector<NavigableNode> NavigableNodes;

	struct AgentInfo
	{
		TerrainAgent* agent;
		AgentLocation location;
	};

	struct ObstacleInfo
	{
		b2AABB box;
	};

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
		pAgent->mpTerrain = this;
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

	WaypointLink* GetWptLink(const LinkAddress& address) 
	{
		WaypointGraph::NodeLinks& links = mWaypointGraph.mLinks[address.linksIndex];
		return &(links.nodeEdges[address.indexInLinks]);
	}
	
	bool FindLinkAddress(int fromWptIndex, int toWptIndex, LinkAddress& linkAddress)
	{
		int linksIndex = mWaypointGraph.getNode(fromWptIndex).linksIndex;

		if (linksIndex >= 0)
		{
			WaypointGraph::NodeLinks& links = mWaypointGraph.mLinks[linksIndex];

			for (size_t i = 0; i < links.nodeEdges.size(); ++i)
			{
				if (links.nodeEdges[i].targetNodeIndex == toWptIndex)
				{
					linkAddress.linkFrom = fromWptIndex;
					linkAddress.linkTo = toWptIndex;
					linkAddress.linksIndex = linksIndex;
					linkAddress.indexInLinks = (int) i;
					return true;
				}
			}

		}

		return false;
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
				WaypointGraph::NodeLinks& links = mWaypointGraph.mLinks[wpt_node.linksIndex];

				for (size_t i=0; i<links.nodeEdges.size(); ++i)
				{
					int neighbor_index = links.nodeEdges[i].targetNodeIndex;
					const WaypointGraph::Node& neighbor_wpt_node = mWaypointGraph.mNodes[neighbor_index];

					float test_radius = wpt_node.GetAABBlockedRadiusForLink(obstacleInfo.box, neighbor_wpt_node, wpt_node.radius, neighbor_wpt_node.origRadius, mUseTangentsForLinks);

					if (test_radius >= 0.0f && (test_radius < wpt_node.origRadius) && (test_radius < links.nodeEdges[i].minRadius))
					{
						links.nodeEdges[i].minRadius = test_radius;
						//wpt_node.radius = test_radius;
					}
				}
			}
		}
	}

	bool Pick(const Vector2D& inPos, int& outWpt, int& outLink)
	{
		outWpt = -1;
		outLink = -1;

		for (size_t i = 0; i < mWaypointGraph.mNodes.size(); ++i)
		{
			const WaypointGraph::Node& wpt_node = mWaypointGraph.mNodes[i];

			if (wpt_node.IsInside(inPos))
			{
				outWpt = (int) i;
				return true;
			}
		}

		for (size_t i=0; i<mWaypointGraph.mLinks.size(); ++i)
		{
			const WaypointGraph::NodeLinks& links = mWaypointGraph.mLinks[i];

			for (size_t j=0; j<links.nodeEdges.size(); ++j)
			{
				int neighbor_index = links.nodeEdges[j].targetNodeIndex;
				const WaypointGraph::Node& wpt_node = mWaypointGraph.mNodes[links.nodeIndex];
				const WaypointGraph::Node& neighbor_wpt_node = mWaypointGraph.mNodes[neighbor_index];

				if (wpt_node.IsInsideLink(inPos, neighbor_wpt_node, &links.nodeEdges[j], mUseTangentsForLinks))
				{
					outWpt = (int) links.nodeIndex;
					outLink = (int) j;

					return true;
				}
			}
		}

		return false;
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
					const WaypointGraph::Edge& link = links.nodeEdges[linkAddress.indexInLinks];
					int neighbor = link.targetNodeIndex;
					const WaypointGraph::Node& node = mWaypointGraph.mNodes[links.nodeIndex];

					if (node.IsStillInsideLink(shape, mWaypointGraph.mNodes[neighbor], &link, mUseTangentsForLinks))
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
							int neighbor = links.nodeEdges[linkAddress.indexInLinks].targetNodeIndex;
							
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

							for (size_t i=0; i<links.nodeEdges.size(); ++i)
							{
								int neighbor_index = links.nodeEdges[i].targetNodeIndex;
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

								for (size_t j=0; j<links.nodeEdges.size(); ++j)
								{
									int neighbor_index = links.nodeEdges[j].targetNodeIndex;
									const WaypointGraph::Node& neighbor_wpt_node = mWaypointGraph.mNodes[neighbor_index];

									LinkAddress testLinkAddress(node.linksIndex, (int)j, node.nodeIndex, neighbor_index);

									if (!IsIn(testLinkAddress, update_links, update_link_count)
										&& node.IsInsideLink(shape, neighbor_wpt_node, &links.nodeEdges[j], mUseTangentsForLinks))
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

							for (size_t j=0; j<links.nodeEdges.size(); ++j)
							{
								const WaypointGraph::Edge& link = links.nodeEdges[j];
								int neighbor_index = link.targetNodeIndex;
								const WaypointGraph::Node& wpt_node = mWaypointGraph.mNodes[links.nodeIndex];
								const WaypointGraph::Node& neighbor_wpt_node = mWaypointGraph.mNodes[neighbor_index];

								if (wpt_node.IsInsideLink(shape, neighbor_wpt_node, &link, mUseTangentsForLinks))
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
						int relevant_nodes[] = { node_links.nodeIndex, node_links.nodeEdges[update_links[0].indexInLinks].targetNodeIndex };

						for (int i = 0; i < 2; ++i)
						{
							WaypointGraph::Node& node = mWaypointGraph.mNodes[relevant_nodes[i]];

							if (node.linksIndex >= 0)
							{
								const WaypointGraph::NodeLinks& links = mWaypointGraph.mLinks[node.linksIndex];

								for (size_t j=0; j<links.nodeEdges.size(); ++j)
								{
									const WaypointGraph::Edge& link = links.nodeEdges[j];
									int neighbor_index = link.targetNodeIndex;
									const WaypointGraph::Node& neighbor_wpt_node = mWaypointGraph.mNodes[neighbor_index];

									LinkAddress testLinkAddress(node.linksIndex, (int) j, node.nodeIndex, neighbor_index);

									if (!IsIn(testLinkAddress, update_links, update_link_count)
										&& node.IsInsideLink(shape, neighbor_wpt_node, &link, mUseTangentsForLinks))
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

					for (size_t i=0; i<links.nodeEdges.size(); ++i)
					{
						const WaypointGraph::Node& neighbor_wpt_node = mWaypointGraph.mNodes[links.nodeEdges[i].targetNodeIndex];

						Vector2D quad[4];

						int quad_point_count = Waypoint::CreateLinkQuad(wpt_node, neighbor_wpt_node, &links.nodeEdges[i], mUseTangentsForLinks, 0.0f, quad);

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
				const WaypointGraph::Edge& link = links.nodeEdges[agentInfo.location.links[i].indexInLinks];
				int neighbor = link.targetNodeIndex;

				Vector2D quad[4];

				int quad_point_count = Waypoint::CreateLinkQuad(mWaypointGraph.mNodes[links.nodeIndex], mWaypointGraph.mNodes[neighbor], &link, mUseTangentsForLinks, 0.0f, quad);
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

	const AgentInfo* FindAgentInfo(const TerrainAgent* pAgent)
	{
		for (size_t i=0; i < mAgentInfos.size(); ++i)
		{
			if (mAgentInfos[i].agent == pAgent)
			{
				return &(mAgentInfos[i]);
			}
		}

		return NULL;
	}
	

	void DrawPath(World& inWorld, const TerrainAgent* pAgent, const Color& inColor, float inLineWidth = 0.7f)
	{
		for (size_t i=0; i < mAgentInfos.size(); ++i)
		{
			AgentInfo& agentInfo = mAgentInfos[i];

			if (agentInfo.agent == pAgent)
			{
				DrawPath(inWorld, agentInfo, inColor, inLineWidth);
				break;
			}
		}
	}

	void DrawPath(World& inWorld, const AgentInfo& agentInfo, const Color& inColor, float inLineWidth = 0.7f)
	{
		if (agentInfo.agent)
		{
			int currToIndex; bool hasTempAvoidPt; Vector2D currTempAvoidancePt;

			const Path* pPath = agentInfo.agent->GetPath(currToIndex, hasTempAvoidPt, currTempAvoidancePt);

			if (pPath)
			{
				DrawPath(inWorld, *pPath, inColor, inLineWidth);

				if (hasTempAvoidPt)
				{
					Renderer& renderer = inWorld.GetRenderer();

					renderer.DrawLine(inWorld.WorldToScreen(agentInfo.agent->GetTerrainShape().pos), 
										inWorld.WorldToScreen(currTempAvoidancePt), inColor, -1.0f, inLineWidth);

					renderer.DrawLine(inWorld.WorldToScreen(currTempAvoidancePt), 
										inWorld.WorldToScreen(pPath->GetPoint(currToIndex)), inColor, -1.0f, inLineWidth);
				}
			}
		}
	}

	void DrawPath(World& inWorld, const Path& path, const Color& inColor, float inLineWidth = 0.7f)
	{
		Renderer& renderer = inWorld.GetRenderer();

		for (int i = 0; i < path.Length(); ++i)
		{
			renderer.DrawCircle(inWorld.WorldToScreen(path.GetPoint(i)), 0.1f, inColor);

			if (i + 1 < path.Length())
			{
				renderer.DrawLine(inWorld.WorldToScreen(path.GetPoint(i)), inWorld.WorldToScreen(path.GetPoint(i+1)), inColor, -1.0f, inLineWidth);
			}
		}
	}

	int IntersectLineFromInside(const AgentLocation& loc, const Vector2D& lineOrig, const Vector2D& lineDir, 
								Vector2D& outSegmentStart, Vector2D& outSegmentEnd, float pointRadius)
	{
		// TODO!!!!! for now only check each wpt or link without using union!!

		for (int i = 0; i < 2; ++i)
		{
			if (loc.links[i].linksIndex >= 0)
			{
				Vector2D quad[4];

				Waypoint::CreateLinkQuad(mWaypointGraph.getNode(loc.links[i].linkFrom), 
					mWaypointGraph.getNode(loc.links[i].linkTo), 
					&(mWaypointGraph.mLinks[loc.links[i].linksIndex].nodeEdges[loc.links[i].indexInLinks]),
					mUseTangentsForLinks, pointRadius, quad);

				float min_t = FLT_MAX;
				float max_t = -FLT_MAX;

				for (int i = 0; i < 4; ++i)
				{
					float t, u;

					if (IntersectLines(lineOrig, lineOrig + lineDir, quad[i], quad[(i+1)%4], t, u)
						&& (u >= 0.0f && u <= 1.0f))
					{
						if (t <= 0.0f)
						{
							min_t = t;
						}

						if (t >= 0.0f)
						{
							max_t = t;
						}
					}
				}

				if (min_t <= 0.0f && max_t >= 0.0f)
				{
					outSegmentStart = (lineOrig + (lineDir * min_t));
					outSegmentEnd = (lineOrig + (lineDir * max_t));

					return 2;
				}
			}
		}


		for (int i = 0; i < 2; ++i)
		{
			if (loc.waypoints[i] >= 0)
			{
				float t, u;
				int inter_count = IntersectLineCircle(lineOrig, lineDir, 
										mWaypointGraph.getNode(loc.waypoints[i]).pos, mWaypointGraph.getNode(loc.waypoints[i]).radius, t, u);

				if (inter_count > 0)
				{
					if (inter_count == 2)
					{
						if ((t >= 0.0f && u < 0.0f)
							|| (t < 0.0f && u >= 0.0f))
						{
							outSegmentStart = (lineOrig + lineDir * t);
							outSegmentEnd = (lineOrig + lineDir * u);

							if (pointRadius != 0.0f)
							{
								if (t >= 0.0f)
									outSegmentStart = outSegmentStart - (lineDir.Normalized() * pointRadius);
								else
									outSegmentStart = outSegmentStart + (lineDir.Normalized() * pointRadius);

								if (u >= 0.0f)
									outSegmentEnd = outSegmentEnd - (lineDir.Normalized() * pointRadius);
								else
									outSegmentEnd = outSegmentEnd + (lineDir.Normalized() * pointRadius);
							}

							return 2;
						}
					}
				}
			}

			
		}
		
		return 0;
	}


	bool IntersectRayWithNavigableNodes(const Vector2D& rayOrig, const Vector2D& rayDir, float pointRadius,
										const NavigableNodes& nodes, Vector2D& outIntersectionPoint, int& outItersectionNode)
	{
		outItersectionNode = -1;
		typedef std::vector<int> Ints;
		Ints unprocessedNodes;
		int unprocessedNodeCount = (int) nodes.size();
		Vector2D nextRayOrig = rayOrig;

		unprocessedNodes.resize(nodes.size());

		for (size_t i = 0; i < nodes.size(); ++i)
			unprocessedNodes[i] = (int) i;

		while (unprocessedNodeCount > 0)
		{
			int containingNode = -1;

			for (size_t i = 0; i < unprocessedNodes.size(); ++i)
			{
				if (unprocessedNodes[i] >= 0)
				{
					if (nodes[unprocessedNodes[i]].Contains(*this, nextRayOrig, pointRadius))
					{
						containingNode = (int) i;
						break;
					}
				}
			}

			if (containingNode != -1)
			{
				outItersectionNode = containingNode;
				nodes[unprocessedNodes[containingNode]].IntersectRay(*this, nextRayOrig, rayDir, pointRadius, outIntersectionPoint);
				nextRayOrig = outIntersectionPoint;

				unprocessedNodes[containingNode] = -1;
				--unprocessedNodeCount;
			}
			else
			{
				break;
			}
		}

		return outItersectionNode != -1;
	}

};

#endif