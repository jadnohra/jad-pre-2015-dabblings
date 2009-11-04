// single file version of my AIGameDev A* implementation
#ifndef ASTAR_H
#define ASTAR_H 

/******************************************************************************
 * This file is part of The AI Sandbox.
 * 
 * Copyright (c) 2008, AiGameDev.com
 * 
 * Credits:         See the PEOPLE file in the base directory.
 * License:         This software may be used for your own personal research
 *                  and education only.  For details, see the LICENSING file.
 *****************************************************************************/

#include <algorithm.h>

enum PathSearchStatus 
{
	InvalidPathSearch = -1, PathSearchNotStarted, PathSearching, PathSearchFound, PathSearchNotFound
};

enum PathSearchInitStatus
{
	InvalidPathSearchInit = -1, PathSearchInitOK, PathSearchInitNoPath, PathSearchInitSameNode, PathSearchIncompatibleGraph
};

// ============================================================================
// AStarAlgorithm
// ----------------------------------------------------------------------------
/// A* implementation.
/// The implementation is static and acts on a state instance.
///
/// State classes must conform to the AStarStateConcept concept.
/// The stock implementation is AStarState.
/// 
// ============================================================================
class AStarAlgorithm 
{
public:

	/**
	 * Execute a Search Step.
	 * 
	 *
	 * @param state            The state to be stepped.
	 * @return                 The status of the search after the step.
	 */
	template<typename State> 
	static PathSearchStatus step(State& state);

protected:

	template<typename State, typename NodeInfo, typename EdgeIterator> 
	static void expand(State& state, NodeInfo& current, EdgeIterator& edgeIter);
};

namespace astar
{

enum NodeStatus 
{ 
	InvalidNodeStatus = -1, NodeIsUnvisited = 0, NodeIsOpen, NodeIsClosed 
};


// ============================================================================
// AStarState
// ----------------------------------------------------------------------------
/// Stock A* state implementation.
///
/// This class fulfills all  AStarStateConcept requirements and is 
/// therefore usable with AStarAlgorithm.
/// 
/// The implementation publicly inherits from GraphView and NodeStateManager,
/// making their functions available.
///
/// GraphView presents a view of the underlying graph, and therefore allows 
/// adding live node support if needed.
///
/// NodeStateManager manages the states of the nodes needed for search
/// (usually know as the open and closed lists).
///
/// There are many ways to manage these states, each with different performance
/// and memory characteristics, therefore it is nice to be able to change the
/// NodeStateManager implementation without touching the rest of the code.
///
/// PathCostEvaluator needs to be implemented by the user, it provides the costs
/// of edges, heuristic estimates and optionally decides how to break ties for 
/// nodes with similar 'f' costs, this can influence the performance of the search
/// and maybe even the resulting path.
///
/// Graph represents the underlying graph, there are many ways to implement a graph,
///	this implementation works with the stock graph implementation found in <terrain/Graph.h>
///
// ============================================================================
template
<	typename Graph
, 	typename PathCostEvaluator
, 	typename GraphView			= GraphView<Graph>
, 	typename NodeStateManager	= NodeStateManager<Graph, typename PathCostEvaluator::CostValue> 
>
class AStarState : public GraphView, public NodeStateManager
{
protected:

	// ----------------------------------------------------------------------------
	// Internal constants.
	// ----------------------------------------------------------------------------

	enum { InvalidNodeIndex = Graph::InvalidIndex };
	enum { InvalidNodeStateIndex = NodeStateManager::InvalidIndex };

	// ----------------------------------------------------------------------------
	// Internal types.
	// ----------------------------------------------------------------------------

	typedef typename NodeStateManager::Index NodeStateIndex;
	typedef typename NodeStateManager::State NodeSearchState;
	
	typedef typename Graph::Index GraphIndex;

	typedef GraphView GraphView;
	typedef NodeStateManager NodeStateManager;

public:

	// ----------------------------------------------------------------------------
	// Public types.
	// ----------------------------------------------------------------------------

	typedef PathCostEvaluator PathCostEvaluator;
	typedef Graph Graph;
	typedef typename Graph::Node Node;
	typedef typename Graph::Edge Edge;
	typedef typename GraphView::LiveNodeIterator LiveNodeIterator;

public:

	// ----------------------------------------------------------------------------
	// Concept type/constant requirements.
	// ----------------------------------------------------------------------------

	enum { IsAdmissibleHeuristic = PathCostEvaluator::IsAdmissibleHeuristic };
	enum { CloseNodeInvalidatesNodeInfo = false };

	typedef NodeStateIndex NodeInfo;
	typedef typename NodeStateManager::ListIterator NodeState;
	
	typedef typename PathCostEvaluator::CostValue CostValue;
	typedef typename GraphView::EdgeIterator EdgeIterator; 

	typedef NodeStateIndex PathIterator;

public:

	// ----------------------------------------------------------------------------
	// Public functions.
	// ----------------------------------------------------------------------------

	AStarState(); 
	AStarState(Graph& graph, PathCostEvaluator& pathCostEvaluator); 
	~AStarState();

	void init(Graph& graph, PathCostEvaluator& pathCostEvaluator);

	/**
	 * Clears the search, calling this after a search is optional but frees some memory.
	 * 
	 * @param doConsolidateMemory		If set, consolidateMemory is called, freeing even more memory.
	 */
	void clearSearch(bool doConsolidateMemory = true);

	/**
	 * Initializes the state search params, allowing it to be searched using AstarAlgorithm::step
	 * 
	 * @param startNode					The search path's starting node
	 * @param goalNode					The search path's goal node
	 * @param expectedVisitedNodeCount	The expected number of visited nodes, this can be retrieved from a previous state using getVisitedNodeCount() per example
	 */
	PathSearchInitStatus initSearch(GraphIndex startNode, GraphIndex goalNode, size_t expectedVisitedNodeCount = 32);
	
	GraphIndex getStartNodeIndex() const;
	GraphIndex getGoalNodeIndex() const;
	bool hasValidSearchNodes() const;

	/**
	 * Frees all curently unsued memory, this might cause slower performance for next searches.
	 */
	void consolidateMemory() { NodeStateManager::consolidateMemory(); }

	/**
	 * Retruns the count of visited nodes
	 */
	size_t getVisitedNodeCount()  { return NodeStateManager::getVisitedNodeCount(); }

	/**
	 * Retruns the cost of the current path
	 */
	CostValue getCurrentPathCost() const { return hasOpenNodes() ? g(getBestOpenNodeInfo()) : (CostValue) -1; }

	/**
	 * Extracts the full current path starting at the goal node. 
	 * The vector is not cleared, new nodes are simply appended to it.
	 * 
	 * @param nodes			A vector of node pointers that will be filled with the current path's nodes
	 * @param includeFirst	A flag indicating if the start node should be included
	 * @param includeLast	A flag indicating if the last node (goal node if the search is complete) should be included 
	 */
	void extractReversePath(vector<GraphIndex>& nodes, bool includeStart = true, bool includeLast = true); 

	/**
	 * Extracts the best path (in reverse) from start node to an arbitrary node.
	 * The vector is not cleared, new nodes are simply appended to it.
	 * 
	 * @param nodes			A vector of node pointers that will be filled with the current path's nodes
	 * @param nodeIndex		The path's end node.
	 */
	void extractReversePathAt(vector<GraphIndex>& nodes, GraphIndex nodeIndex); 
	
	/**
	 * returns an iterator that begins at the goal node
	 */
	PathIterator beginReversePath() const;

	/**
	 * Returns the current iterator node and advances the iterator.
	 * 
	 * @param iter			The path iterator
	 * @return				A pointer to current path Node or NULL
	 */
	const Node* nextPathNode(PathIterator& iter) const;

	/**
	 * Returns true if the iterator still has nodes.
	 */
	bool isAtEnd(PathIterator& iter) const;

	/**
	 * Finds the status of a node.
	 * 
	 * @param nodeIndex			index of the node.
	 * @param pResult			extracted state (optional)
	 * @return					the node's status
	 */
	NodeStatus getNodeStatus(const GraphIndex& nodeIndex, NodeState* pResult = NULL) const
	{
		NodeState resultHolder;

		return NodeStateManager::getNodeStatus(nodeIndex, pResult ? *pResult : resultHolder);
	}

public:

	// ----------------------------------------------------------------------------
	// Accessors.
	// ----------------------------------------------------------------------------

	Graph* getGraph() const;
	PathCostEvaluator* getPathCostEvaluator();

	Graph& graph() const;
	PathCostEvaluator& pathCostEvaluator();

	const Node& getStartNode() const;
	const Node& getGoalNode() const;

	const Node& getNode(const GraphIndex& nodeIndex) const;

public:

	// ----------------------------------------------------------------------------
	// Concept function requirements. 
	// ----------------------------------------------------------------------------

	/**
	 * Copy operator allowing states to be copied.
	 */
	AStarState& operator=(const AStarState& ref);

	bool hasOpenNodes() const;
	NodeInfo getBestOpenNodeInfo() const;
	
	bool isGoalNode(const NodeInfo& nodeInfo) const;
	CostValue g(const NodeInfo& nodeInfo) const;
	
	EdgeIterator getBestOpenNodeEdges();
	bool isAtEnd(EdgeIterator& iter);
	bool hasEdge(EdgeIterator& iter);
	void nextEdge(EdgeIterator& iter);
	
	NodeStatus getEdgeTargetState(EdgeIterator& iter, NodeState& result);
	
	CostValue g(const NodeState& state) const;
	CostValue h(const NodeState& state) const;
	CostValue f(const NodeState& state) const;
	
	CostValue getEdgePathCost(EdgeIterator& iter, const NodeInfo& source);

	void updateOpenNode(EdgeIterator& iter, NodeState& nodeState, const NodeInfo& newPathPredecessor, const CostValue& newG);
	void openClosedNode(EdgeIterator& iter, NodeState& nodeState, const NodeInfo& predecessor, const CostValue& g);
	void openUnvisitedNode(EdgeIterator& iter, NodeState& nodeState, const NodeInfo& pathPredecessor, const CostValue& g);
	void closeBestOpenNode();
	
protected:

	// ----------------------------------------------------------------------------
	// Internal functions.
	// ----------------------------------------------------------------------------

	void openUnvisitedNode(const GraphIndex& nodeIndex, const NodeInfo& pathPredecessor, const CostValue& g);

protected:

	// ----------------------------------------------------------------------------
	// Internal Node and Edge getters.
	// ----------------------------------------------------------------------------

	const Node& node(const GraphIndex& nodeIndex) const { return GraphView::node(*getGraph(), nodeIndex); }
	const Node& node(const NodeState& result) { return nodeFromNodeInfo((*(result))->nodeIndex); }
	const Node& targetNode(const EdgeIterator& iter) { return node(getTargetNodeIndex(edge(iter))); }
	const Edge& edge(const EdgeIterator& iter) { return GraphView::edge(*getGraph(), iter); }
	const GraphIndex edgeIndex(const EdgeIterator& iter) { return GraphView::edgeIndex(iter); }
	GraphIndex getTargetNodeIndex(const Edge& edge) { return (GraphIndex) edge; }
	GraphIndex getTargetNodeIndex(const EdgeIterator& iter) { return (GraphIndex) ( getTargetNodeIndex(edge(iter)) ); }
	const Node& nodeFromNodeInfo(const NodeStateIndex& nodeInfo) const { return nodeFromStateIndex(nodeInfo); }
	const Node& nodeFromStateIndex(const NodeStateIndex& nodeStateIndex) const { return node(NodeStateManager::nodeIndex(nodeStateIndex)); }
	GraphIndex nodeIndex(const NodeStateIndex& nodeStateIndex) { return NodeStateManager::nodeIndex(nodeStateIndex); }
	
protected:

	Graph* m_pGraph;
	PathCostEvaluator* m_pPathCostEvaluator;

	GraphIndex m_startNodeIndex;
	GraphIndex m_goalNodeIndex;
};


// ============================================================================
// PathCostEvalConcept
// ----------------------------------------------------------------------------
// The Concept of a PathCostEval class. 
// Describes the minimum needed type declarations and functions needed.
// 
// ============================================================================
class PathCostEvalConcept
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

	/**
	 * Gets the Path cost 'g' between 2 Nodes connected by an Edge
	 *
	 * @return				The path cost ('g')
	 */
	template<typename Node, typename Edge, typename EdgeIndex>
	CostValue getEdgeCost(const Node& source, const Edge& edge, EdgeIndex edgeIndex);
	
	/**
	 * Gets the Heuristic Path cost 'h' between 2 Nodes connected
	 *
	 * @return				The heuristic path cost ('h')
	 */
	template<typename Node>
	CostValue getHeuristicCost(const Node& from, const Node& to);

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
			return stateManager.h(comp) < stateManager.h(ref);
		}
		
		return false;
	}
};


// ============================================================================
// GraphView
// ----------------------------------------------------------------------------
/// This class fulfills all GraphViewConcept requirements and is 
/// therefore usable with AStarState.
///
/// A GraphView presents a view of the graph on which the AStarState acts.
///
/// This implementation relies on the underlying graph to provide it's 
/// functionality and provides no live node support.
///
// ============================================================================
template<typename Graph>
class GraphView
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
	const Edge& edge(Graph& graph, const EdgeIterator& edgeIter) { return graph.getEdge(*(edgeIter.m_curr)); }
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


//NOTE this should be replaced by a design based on allocators.
class GrowthConcept
{
public:

	/**
	 * Returns the new size
	 * 
	 *
	 * @param currSize		   The current size
	 * @param neededSize	   The minimum needed size
	 * @return                 The new size
	 */
	static size_t grow(size_t currSize, size_t neededSize);
};

// ============================================================================
// BlockGrowth
// ----------------------------------------------------------------------------
/// Growth Policy implementation that grows based on fixed size blocks.
/// 
// ============================================================================
template<size_t BlockSize>
class BlockGrowth 
{
public:

	/**
	 * Returns the new size
	 */
	static size_t grow(size_t currSize, size_t neededSize) 
	{
		return std::max(neededSize, currSize + BlockSize);
	}
};


// ============================================================================
// LiveGraphView
// ----------------------------------------------------------------------------
/// This class fulfills all GraphViewConcept requirements and is 
/// therefore usable with AStarState.
///
/// A GraphView presents a view of the graph on which the AStarState acts.
///
/// This implementation supports adding a live start and a live goal node,
/// along with edges from the start node and edges to the goal node.
///
/// The start and goal node Indices are constant and take the last 2 possible 
/// values from the Node Index data type.
///
/// Live edges are stored in a map and injected into the edge iterator.
/// 
/// There is a performance penalty associated with live support in 
/// form of map lookups when creating edge iterators
/// and switch statements when looking up nodes.
///
/// EdgesMap is a templated type, in order to allow using other map 
/// implementations such as hash_map.
///
// ============================================================================
template
<	typename Graph
,	typename EdgesMap = map<Graph::Index, Graph::Edges> 
>
class LiveGraphView
{
public:

	// ----------------------------------------------------------------------------
	// Type aliases.
	// ----------------------------------------------------------------------------

	typedef typename Graph::Node Node;
	typedef typename Graph::Edge Edge;
	typedef typename Graph::Index Index;
	typedef typename Graph::Edges Edges;
	typedef typename Graph::EdgeIndices EdgeIndices;

	// ----------------------------------------------------------------------------
	// Constants.
	// ----------------------------------------------------------------------------

	/// constant indices used for the live nodes
	enum 
	{ 
		LiveStartNodeIndex = (Index) -3, 
		LiveGoalNodeIndex = (Index) -2 
	};

public:

	// ----------------------------------------------------------------------------
	// Concept types.
	// ----------------------------------------------------------------------------

	struct EdgeIterator;
	struct LiveNodeIterator;

public:

	// ----------------------------------------------------------------------------
	// Concept functions.
	// ----------------------------------------------------------------------------

	bool isCompatible(Graph& graph);

	const Node& node(Graph& graph, const Index& nodeIndex) const;
	const Edge& edge(Graph& graph, const EdgeIterator& edgeIter);
	const Index edgeIndex(const EdgeIterator& edgeIter);

	EdgeIterator edgeIterator(Graph& graph, const Index& nodeIndex);

	bool isAtEnd(EdgeIterator& iter);

	bool hasEdge(EdgeIterator& iter);
	void nextEdge(EdgeIterator& iter);
	
	LiveNodeIterator liveNodeIterator() const;

	const Node* getLiveNode(LiveNodeIterator& iter) const;
	Index getLiveNodeIndex(LiveNodeIterator& iter) const;

	void nextLiveNode(LiveNodeIterator& iter) const;

public:

	// ----------------------------------------------------------------------------
	// Public live node and edge functions.
	// ----------------------------------------------------------------------------

	Node& liveStartNode();
	Node& liveGoalNode();

	Edges* getLiveEdges(const Index& nodeIndex, bool createIfNotFound);

	void clearLiveEdges();
	void clearStartLiveEdges();
	void clearGoalLiveEdges();

protected:

	const EdgeIndices& edgeIndices(Graph& graph, const Index& nodeIndex); 
	Edges* liveEdges(const Index& nodeIndex);

	//NOTE to enable search states with live node support to be copied this has to be implemented.
	LiveGraphView<Graph, EdgesMap>& operator=(const LiveGraphView<Graph, EdgesMap>& ref);

public:

	// ----------------------------------------------------------------------------
	// Concept types implementations.
	// ----------------------------------------------------------------------------

	struct EdgeIterator 
	{
		typedef typename Edges::const_iterator EdgeIter;
		typedef typename EdgeIndices::const_iterator EdgeIndexIter;

		EdgeIterator(const Edges* pEdges, const Index& nodeIndex);
		EdgeIterator(const EdgeIndices* pEdgeIndices, const Index& nodeIndex, bool dummy);

		bool m_isLiveEdges;
		Index m_nodeIndex;
		
		union
		{
			struct 
			{
				const Edges* m_pEdges;
				EdgeIter m_curr;
			};

			struct
			{
				const EdgeIndices* m_pEdgeIndices;
				EdgeIndexIter m_currIndex;
			};
		};
	};

	struct LiveNodeIterator
	{
		Index m_nodeIndex;

		LiveNodeIterator(Index nodeIndex);
	};

protected:

	Node m_startNode;
	Node m_goalNode;
	EdgesMap m_liveEdges;
};

// ============================================================================
// NodeStateManager
// ----------------------------------------------------------------------------
/// This class fulfills all NodeStateManagerConcept requirements and is 
/// therefore usable with AStarState.
///
/// The GrowthPolicy is used to grow a vector containing the states of visited 
/// (open of closed) nodes.
///
// ============================================================================
template 
<	typename Graph
,	typename CostValue
,	typename GrowthPolicy = BlockGrowth<16>
>
class NodeStateManager : protected GrowthPolicy
{
public:

	// ----------------------------------------------------------------------------
	// Concept Type/Constant requirements.
	// ----------------------------------------------------------------------------

	typedef size_t Index;

	enum { InvalidIndex = (Index) -1 };

protected:

	// ----------------------------------------------------------------------------
	// Internal types and aliases.
	// ----------------------------------------------------------------------------
	
	typedef typename Graph::Index GraphIndex;
	typedef CostValue CostValue;
	typedef std::map<GraphIndex, Index> IndexByNodeIndexMap;

	// ----------------------------------------------------------------------------
	// Internal constants.
	// ----------------------------------------------------------------------------

	enum { InvalidNodeIndex = Graph::InvalidIndex };

public:

	// ----------------------------------------------------------------------------
	// Concept type implementations.
	// ----------------------------------------------------------------------------

	struct State 
	{
		GraphIndex nodeIndex;
		CostValue g; // cost of this node + it's predecessors
		CostValue h; // heuristic estimate of distance to goal
		CostValue f; // sum of cumulative cost of predecessors and self and heuristic
		Index pathPredecessor; //the best current path predecessor
		NodeStatus status;

		State(); 
	};

	struct ListIterator
	{
		typename IndexByNodeIndexMap::const_iterator iter;
	};

protected:

	// ----------------------------------------------------------------------------
	// Internal types.
	// ----------------------------------------------------------------------------

	typedef vector<State> States;
	typedef vector<Index> StateIndices;
	typedef StateIndices OpenHeap;

public:

	// ----------------------------------------------------------------------------
	// Public 'opaque state' access functions.
	// ----------------------------------------------------------------------------

	GraphIndex nodeIndex(Index stateIndex) const { return state(stateIndex).nodeIndex; }
	CostValue g(Index stateIndex) const { return state(stateIndex).g; }
	CostValue h(Index stateIndex) const { return state(stateIndex).h; }
	CostValue f(Index stateIndex) const { return state(stateIndex).f; }
	Index pathPredecessor(Index stateIndex) const { return state(stateIndex).pathPredecessor; }

public:

	// ----------------------------------------------------------------------------
	// Concept functions.
	// ----------------------------------------------------------------------------

	/// Returns the State found at an index
	State& state(Index stateIndex) { return m_nodeSearchStates[stateIndex]; }

	/// Returns the State found at an index
	const State& state(Index stateIndex) const { return m_nodeSearchStates[stateIndex]; }

	/// Returns the State index pointed to by the iterator
	Index stateIndex(const ListIterator& iter) const { return iter.iter->second; }

	/// Returns the index of the Node related to a sate
	GraphIndex nodeIndex(const State& status) const { return status.nodeIndex; }

	/// Returns the 'g' cost of a sate
	CostValue g(const State& state) const { return state.g; }
	
	/// Returns the 'h' cost of a sate
	CostValue h(const State& state) const { return state.h; }
	
	/// Returns the 'f' cost of a sate
	CostValue f(const State& state) const { return state.f; }

	/// Returns the size of the open list
	size_t openListSize() const { return m_openHeap.size(); }

	/// Returns the size of the closed list
	size_t closedListSize() const { return m_closedListSize; }

	/**
	 * Retruns the count of visited nodes
	 */
	Index getVisitedNodeCount() const { return m_usedNodeSearchStateCount; }

	/**
	 * Returns true if there are still open nodes left
	 */
	bool hasOpenNodes() const { return !m_openHeap.empty(); }

	/**
	 *  Returns the index the best open node's state
	 */
	Index getBestStateIndex() const { return m_openHeap.front(); }

	/**
	 * Frees all curently unsued memory
	 */
	void consolidateMemory() { m_nodeSearchStates.resize(m_usedNodeSearchStateCount); } 

	/**
	 * Opens an unvisited node
	 */
	template<typename PathCostEvaluator>
	void openUnvisited(PathCostEvaluator& pathCostEval, GraphIndex nodeIndex, Index pathPredecessor, CostValue g, CostValue h);
	
	/**
	 * Updates an open node with a new 'g' cost and a new path parent
	 */
	template<typename PathCostEvaluator>
	void updateOpen(PathCostEvaluator& pathCostEval, ListIterator& openListIter, Index newPathPredecessor, CostValue newG);

	/**
	 * Opens a closed node (only needed if IsAdmissibleHeuristic is false)
	 */
	template<typename PathCostEvaluator>
	void openClosed(PathCostEvaluator& pathCostEval, ListIterator& closedListIter, Index newPathPredecessor, CostValue newG);
	
	/**
	 * Closes the current best node
	 */
	template<typename PathCostEvaluator>
	void closeBestOpen(PathCostEvaluator& pathCostEval);

	/**
	 * Functional copy operator allowing states to be copied.
	 */
	NodeStateManager& operator=(const NodeStateManager& ref);
	
	/**
	 * Returns the status of a Node.
	 */
	NodeStatus getNodeStatus(const GraphIndex& nodeIndex, ListIterator& result) const;
	
	/**
	 * Extracts the nodes in the open list, used for debugging
	 */
	void extractOpenList(vector<GraphIndex>& nodes) const;
	

	/**
	 * Extracts the nodes in the closed list, used for debugging
	 */
	void extractClosedList(vector<GraphIndex>& nodes) const;
	
	/**
	 * Initializes the container perparing it for a new search
	 */
	void initSearch(size_t expectedVisitedNodeCount);

public:

	// ----------------------------------------------------------------------------
	// Public functions.
	// ----------------------------------------------------------------------------

	NodeStateManager();
	~NodeStateManager();
	
protected:

	Index newState();
	
	template<typename PathCostEvaluator>
	void reinsertOpenState(PathCostEvaluator& pathCostEval, Index stateIndex, Index newPathPredecessor, CostValue newG);

	OpenHeap::iterator findInOpenList(GraphIndex nodeIndex);
	
protected:

	// ----------------------------------------------------------------------------
	// Internal classes.
	// ----------------------------------------------------------------------------

	template<typename PathCostEvaluator>
	class OpenList_HeapCompare_f 
	{
	public:

		OpenList_HeapCompare_f(NodeStateManager& stateContainer, PathCostEvaluator& pathCostEval);

		bool operator() (const Index& x, const Index& y) const;
		
	protected:

		NodeStateManager& m_stateManager;
		PathCostEvaluator& m_pathCostEval;
	};

protected:
	
	Index m_usedNodeSearchStateCount;
	States m_nodeSearchStates;
	
	size_t m_closedListSize;

	IndexByNodeIndexMap m_stateIndexByNodeIndexMap;
	OpenHeap m_openHeap;
};

#include "AStar.inl"

}

#endif