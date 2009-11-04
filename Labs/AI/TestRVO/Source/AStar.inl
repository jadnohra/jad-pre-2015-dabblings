#ifndef ASTAR_H
#error "Include from parent header only."
#else

template<bool IsAdmissibleHeuristic> 
class AStarAlgorithmHeuristicHelper;

template<> 
class AStarAlgorithmHeuristicHelper<true> 
{
public:

	template<typename State, typename EdgeIterator, typename NodeStateSearchResultCache, typename NodePtr, typename CostValue>
	static inline void onNeighborImproved(State& state, 
								EdgeIterator& edgeIter, 
								NodeStateSearchResultCache& stateSearchResultCache, 
								NodePtr& current, 
								CostValue& cost) 
	{
	}
};

template<> 
class AStarAlgorithmHeuristicHelper<false> 
{
public:

	template<typename State, typename EdgeIterator, typename NodeStateSearchResultCache, typename NodePtr, typename CostValue>
	static inline void onNeighborImproved(State& state, 
								EdgeIterator& edgeIter, 
								NodeStateSearchResultCache& stateSearchResultCache, 
								NodePtr& current, 
								CostValue& cost) 
	{
		state.openClosedNode(edgeIter, stateSearchResultCache, current, cost);
	}
};


template<typename State> 
PathSearchStatus AStarAlgorithm::step(State& state) 
{
	if (!state.hasOpenNodes()) 
	{
		return PathSearchNotFound;
	}

	State::NodeInfo current = state.getBestOpenNodeInfo();	
	if (state.isGoalNode(current))
	{
		return PathSearchFound;
	}

	State::EdgeIterator edgeIter = state.getBestOpenNodeEdges();
	if (!State::CloseNodeInvalidatesNodeInfo)
	{
		state.closeBestOpenNode();
	}

	if (!state.isAtEnd(edgeIter)) 
	{
		expand(state, current, edgeIter);
	}

	if (State::CloseNodeInvalidatesNodeInfo)
	{
		state.closeBestOpenNode();
	}

	return PathSearching;
}


template<typename State, typename NodeInfo, typename EdgeIterator> 
void AStarAlgorithm::expand(State& state, NodeInfo& current, EdgeIterator& edgeIter) 
{
	typedef typename State::CostValue CostValue;

	const CostValue gCurrent = state.g(current);
	State::NodeState stateResult;

	while (state.hasEdge(edgeIter)) 
	{
		const CostValue cost = gCurrent + state.getEdgePathCost(edgeIter, current);

		switch (state.getEdgeTargetState(edgeIter, stateResult)) 
		{
			case NodeIsOpen: 
			{
				if (state.g(stateResult) > cost) 
				{
					state.updateOpenNode(edgeIter, stateResult, current, cost);
				}
				break;
			}

			case NodeIsClosed: 
			{
				if (!State::IsAdmissibleHeuristic) 
				{
					if (state.g(stateResult) > cost) 
					{
						//from closed to open and update
						AStarAlgorithmHeuristicHelper<State::IsAdmissibleHeuristic>::onNeighborImproved(state, edgeIter, stateResult, current, cost);
					}
				}
				break;
			}

			default: 
			{
				//add to open and update
				state.openUnvisitedNode(edgeIter, stateResult, current, cost);
				break;
			}
		}

		state.nextEdge(edgeIter);
	}
}



template <typename G, typename PCE, typename GV, typename NSM> 
AStarState<G, PCE, GV, NSM>::AStarState() 
:	m_pGraph(NULL)
,	m_pPathCostEvaluator(NULL)
,	m_startNodeIndex(InvalidNodeIndex)
,	m_goalNodeIndex(InvalidNodeIndex)
{
}

template <typename G, typename PCE, typename GV, typename NSM> 
AStarState<G, PCE, GV, NSM>::AStarState(Graph& graph, PathCostEvaluator& pathCostEvaluator) 
:	m_pGraph(NULL)
,	m_pPathCostEvaluator(NULL)
,	m_startNodeIndex(InvalidNodeIndex)
,	m_goalNodeIndex(InvalidNodeIndex)
{
	init(graph, pathCostEvaluator);
}

template <typename G, typename PCE, typename GV, typename NSM> 
AStarState<G, PCE, GV, NSM>::~AStarState() 
{
}

template <typename G, typename PCE, typename GV, typename NSM> 
void AStarState<G, PCE, GV, NSM>::init(Graph& graph, PathCostEvaluator& pathCostEvaluator)
{
	m_pGraph = &graph;
	m_pPathCostEvaluator = &pathCostEvaluator;
}


template <typename G, typename PCE, typename GV, typename NSM> 
AStarState<G, PCE, GV, NSM>& AStarState<G, PCE, GV, NSM>::operator=(const AStarState& ref) 
{
	this->m_pGraph = ref.m_pGraph;
	this->m_pPathCostEvaluator = ref.m_pPathCostEvaluator;
	this->m_startNodeIndex = ref.m_startNodeIndex;
	this->m_goalNodeIndex = ref.m_goalNodeIndex;

	((NodeStateManager&) *this) = (NodeStateManager&) ref;
	((GraphView&) *this) = (GraphView&) ref;

	return *this;
}

template <typename G, typename PCE, typename GV, typename NSM> 
void AStarState<G, PCE, GV, NSM>::clearSearch(bool freeAdditionalMemory) 
{
	initSearch(InvalidNodeIndex, InvalidNodeIndex, getVisitedNodeCount());

	if (freeAdditionalMemory)
		consolidateMemory();
}

template <typename G, typename PCE, typename GV, typename NSM> 
PathSearchInitStatus AStarState<G, PCE, GV, NSM>::initSearch(GraphIndex startNode, GraphIndex goalNode, size_t expectedVisitedNodeCount) 
{
	if (!GraphView::isCompatible(*m_pGraph))
	{
		return PathSearchIncompatibleGraph;
	}

	m_startNodeIndex = startNode;
	m_goalNodeIndex = goalNode;

	NodeStateManager::initSearch(expectedVisitedNodeCount);

	if (m_startNodeIndex == InvalidNodeIndex
		||  m_goalNodeIndex == InvalidNodeIndex)
	{
		return PathSearchInitNoPath;
	}

	if (m_startNodeIndex == m_goalNodeIndex
		||  node(m_startNodeIndex) == node(m_goalNodeIndex))
	{
		return PathSearchInitSameNode;
	}
	else 
	{
		openUnvisitedNode(m_startNodeIndex, InvalidNodeStateIndex, 0);
	}

	return PathSearchInitOK;
}

template <typename G, typename PCE, typename GV, typename NSM> 
void AStarState<G, PCE, GV, NSM>::extractReversePath(vector<GraphIndex>& nodes, bool includeStart, bool includeLast) 
{
	size_t index = 0;

	if (hasOpenNodes()) {

		NodeInfo nodeInfo = hasOpenNodes() ? getBestOpenNodeInfo() : InvalidNodeStateIndex;
		
		while (nodeInfo != InvalidNodeStateIndex) {

			if (index != 0 || includeLast)
			{
				nodes.push_back(nodeIndex(nodeInfo));
				nodeInfo = NodeStateManager::pathPredecessor(nodeInfo); 
			}

			++index;
		}
	}

	if (!includeStart && index > 0)
	{
		nodes.pop_back();
	}
}

template <typename G, typename PCE, typename GV, typename NSM> 
void AStarState<G, PCE, GV, NSM>::extractReversePathAt(vector<GraphIndex>& nodes, GraphIndex node)
{
	ListIterator nodeStateIt;

	if (NodeStateManager::getNodeStatus(node, nodeStateIt) != NodeIsUnvisited)
	{
		NodeInfo nodeInfo = NodeStateManager::stateIndex(nodeStateIt);

		while (nodeInfo != InvalidNodeStateIndex) {

			nodes.push_back(nodeIndex(nodeInfo));
			nodeInfo = NodeStateManager::pathPredecessor(nodeInfo); 
		}
	}
}


template <typename G, typename PCE, typename GV, typename NSM> 
typename AStarState<G, PCE, GV, NSM>::PathIterator AStarState<G, PCE, GV, NSM>::beginReversePath() const
{
	return PathIterator(hasOpenNodes() ? getBestOpenNodeInfo() : InvalidNodeStateIndex);
}

template <typename G, typename PCE, typename GV, typename NSM> 
typename const G::Node* AStarState<G, PCE, GV, NSM>::nextPathNode(PathIterator& iter) const
{
	if (iter == InvalidNodeStateIndex) {

		return NULL;
	}

	const Node& retNode = nodeFromStateIndex(iter);
	iter = NodeStateManager::pathPredecessor(iter);

	return &retNode;
}

template <typename G, typename PCE, typename GV, typename NSM> 
bool AStarState<G, PCE, GV, NSM>::isAtEnd(PathIterator& iter) const
{
	return iter == InvalidNodeStateIndex;
}

template <typename G, typename PCE, typename GV, typename NSM> 
bool AStarState<G, PCE, GV, NSM>::hasOpenNodes() const
{ 
	return NodeStateManager::hasOpenNodes(); 
}
template <typename G, typename PCE, typename GV, typename NSM> 
typename AStarState<G, PCE, GV, NSM>::NodeInfo AStarState<G, PCE, GV, NSM>::getBestOpenNodeInfo() const
{ 
	return NodeStateManager::getBestStateIndex(); 
}
	
template <typename G, typename PCE, typename GV, typename NSM> 
bool AStarState<G, PCE, GV, NSM>::isGoalNode(const NodeInfo& nodeInfo) const
{ 
	return NodeStateManager::nodeIndex(nodeInfo) == m_goalNodeIndex; 
}

template <typename G, typename PCE, typename GV, typename NSM> 
typename PCE::CostValue AStarState<G, PCE, GV, NSM>::g(const NodeInfo& nodeInfo) const
{ 
	return NodeStateManager::g(nodeInfo); 
}
	
template <typename G, typename PCE, typename GV, typename NSM> 
typename GV::EdgeIterator AStarState<G, PCE, GV, NSM>::getBestOpenNodeEdges() 
{ 
	return edgeIterator(graph(), NodeStateManager::nodeIndex(getBestOpenNodeInfo())); 
}

template <typename G, typename PCE, typename GV, typename NSM> 
bool AStarState<G, PCE, GV, NSM>::isAtEnd(EdgeIterator& iter) 
{ 
	return GraphView::isAtEnd(iter); 
}

template <typename G, typename PCE, typename GV, typename NSM> 
bool AStarState<G, PCE, GV, NSM>::hasEdge(EdgeIterator& iter) 
{ 
	return GraphView::hasEdge(iter); 
}

template <typename G, typename PCE, typename GV, typename NSM> 
void AStarState<G, PCE, GV, NSM>::nextEdge(EdgeIterator& iter) 
{ 
	GraphView::nextEdge(iter); 
}
	
template <typename G, typename PCE, typename GV, typename NSM> 
NodeStatus AStarState<G, PCE, GV, NSM>::getEdgeTargetState(EdgeIterator& iter, NodeState& result) 
{ 
	return NodeStateManager::getNodeStatus(getTargetNodeIndex(edge(iter)), result);
}
	
template <typename G, typename PCE, typename GV, typename NSM> 
typename PCE::CostValue AStarState<G, PCE, GV, NSM>::g(const NodeState& state) const
{ 
	return NodeStateManager::g(NodeStateManager::state(NodeStateManager::stateIndex(state))); 
}

template <typename G, typename PCE, typename GV, typename NSM> 
typename PCE::CostValue AStarState<G, PCE, GV, NSM>::h(const NodeState& state)  const
{ 
	return NodeStateManager::h(NodeStateManager::state(NodeStateManager::stateIndex(state))); 
}

template <typename G, typename PCE, typename GV, typename NSM> 
typename PCE::CostValue AStarState<G, PCE, GV, NSM>::f(const NodeState& state)  const
{ 
	return NodeStateManager::f(NodeStateManager::state(NodeStateManager::stateIndex(state))); 
}

template <typename G, typename PCE, typename GV, typename NSM> 
typename PCE::CostValue AStarState<G, PCE, GV, NSM>::getEdgePathCost(EdgeIterator& iter, const NodeInfo& source) 
{
	return pathCostEvaluator().getEdgeCost(nodeFromNodeInfo(source), edge(iter), edgeIndex(iter));
}

template <typename G, typename PCE, typename GV, typename NSM> 
void AStarState<G, PCE, GV, NSM>:: updateOpenNode(EdgeIterator& iter, NodeState& nodeState, const NodeInfo& newPathPredecessor, const CostValue& newG) 
{
	NodeStateManager::updateOpen(pathCostEvaluator(), nodeState, newPathPredecessor, newG);
}

template <typename G, typename PCE, typename GV, typename NSM> 
void AStarState<G, PCE, GV, NSM>::openClosedNode(EdgeIterator& iter, NodeState& nodeState, const NodeInfo& predecessor, const CostValue& g) 
{
	NodeStateManager::openClosed(pathCostEvaluator(), nodeState, predecessor, g);
}

template <typename G, typename PCE, typename GV, typename NSM> 
void AStarState<G, PCE, GV, NSM>::openUnvisitedNode(EdgeIterator& iter, NodeState& nodeState, const NodeInfo& pathPredecessor, const CostValue& g) 
{
	openUnvisitedNode(getTargetNodeIndex(edge(iter)), pathPredecessor, g);
}

template <typename G, typename PCE, typename GV, typename NSM> 
void AStarState<G, PCE, GV, NSM>::closeBestOpenNode() 
{
	NodeStateManager::closeBestOpen(pathCostEvaluator());
}

template <typename G, typename PCE, typename GV, typename NSM> 
void AStarState<G, PCE, GV, NSM>::openUnvisitedNode(const GraphIndex& nodeIndex, const NodeInfo& pathPredecessor, const CostValue& g) 
{
	CostValue h = pathCostEvaluator().getHeuristicCost(node(nodeIndex), getGoalNode());
	NodeStateManager::openUnvisited(pathCostEvaluator(), nodeIndex, pathPredecessor, g, h);
}

template <typename G, typename PCE, typename GV, typename NSM> 
G* AStarState<G, PCE, GV, NSM>::getGraph() const 
{ 
	return m_pGraph; 
}

template <typename G, typename PCE, typename GV, typename NSM> 
PCE* AStarState<G, PCE, GV, NSM>::getPathCostEvaluator() 
{ 
	return m_pPathCostEvaluator; 
}

template <typename G, typename PCE, typename GV, typename NSM> 
G& AStarState<G, PCE, GV, NSM>::graph() const 
{ 
	return *m_pGraph; 
}

template <typename G, typename PCE, typename GV, typename NSM> 
PCE& AStarState<G, PCE, GV, NSM>::pathCostEvaluator() 
{ 
	return *m_pPathCostEvaluator; 
}

template <typename G, typename PCE, typename GV, typename NSM> 
typename G::Index AStarState<G, PCE, GV, NSM>::getStartNodeIndex() const
{ 
	return m_startNodeIndex; 
}

template <typename G, typename PCE, typename GV, typename NSM> 
typename G::Index AStarState<G, PCE, GV, NSM>::getGoalNodeIndex() const
{ 
	return m_goalNodeIndex; 
}

template <typename G, typename PCE, typename GV, typename NSM> 
bool AStarState<G, PCE, GV, NSM>::hasValidSearchNodes() const
{ 
	return m_startNodeIndex != InvalidNodeIndex && m_goalNodeIndex != InvalidNodeIndex; 
}

template <typename G, typename PCE, typename GV, typename NSM> 
typename const G::Node& AStarState<G, PCE, GV, NSM>::getStartNode() const
{ 
	return node(m_startNodeIndex); 
}
template <typename G, typename PCE, typename GV, typename NSM> 
typename const G::Node& AStarState<G, PCE, GV, NSM>::getGoalNode() const
{ 
	return node(m_goalNodeIndex); 
}

template <typename G, typename PCE, typename GV, typename NSM> 
typename const G::Node& AStarState<G, PCE, GV, NSM>::getNode(const GraphIndex& nodeIndex) const
{ 
	return node(nodeIndex); 
}


template<typename Graph, typename EdgesMap>
bool LiveGraphView<Graph, EdgesMap>::isCompatible(Graph& graph) { 
	
	Index lastIndex = graph.getSize() - 1; 
	Index preLastIndex = graph.getSize() - 2; 

	return (lastIndex != LiveStartNodeIndex && preLastIndex != LiveStartNodeIndex
			&& lastIndex != LiveGoalNodeIndex && preLastIndex != LiveGoalNodeIndex);
}

template<typename Graph, typename EdgesMap>
const typename Graph::Node& LiveGraphView<Graph, EdgesMap>::node(Graph& graph, const Index& nodeIndex) const
{ 
	switch (nodeIndex)
	{
		case LiveStartNodeIndex: return m_startNode;
		case LiveGoalNodeIndex: return m_goalNode;
	}

	return graph.getNode(nodeIndex); 
}

template<typename Graph, typename EdgesMap>
const typename Graph::Edge& LiveGraphView<Graph, EdgesMap>::edge(Graph& graph, const EdgeIterator& edgeIter)
{ 
	if (edgeIter.m_isLiveEdges)
		return (*(edgeIter.m_curr)); 
	else
		return graph.getEdge(*(edgeIter.m_currIndex)); 
}

template<typename Graph, typename EdgesMap>
const typename Graph::Index LiveGraphView<Graph, EdgesMap>::edgeIndex(const EdgeIterator& edgeIter)
{ 
	if (edgeIter.m_isLiveEdges)
		return Graph::InvalidIndex; 
	else
		return *(edgeIter.m_currIndex); 
}

template<typename Graph, typename EdgesMap>
typename LiveGraphView<Graph, EdgesMap>::EdgeIterator LiveGraphView<Graph, EdgesMap>::edgeIterator(Graph& graph, const Index& nodeIndex) 
{ 
	switch (nodeIndex)
	{
		case LiveStartNodeIndex: return EdgeIterator(liveEdges(nodeIndex), nodeIndex);
		case LiveGoalNodeIndex: return EdgeIterator((Edges*) NULL, nodeIndex);
	}

	return EdgeIterator(&(edgeIndices(graph, nodeIndex)), nodeIndex, true); 
}

template<typename Graph, typename EdgesMap>
bool LiveGraphView<Graph, EdgesMap>::isAtEnd(EdgeIterator& iter) 
{ 
	return (iter.m_pEdges == NULL) || !hasEdge(iter); 
}

template<typename Graph, typename EdgesMap>
bool LiveGraphView<Graph, EdgesMap>::hasEdge(EdgeIterator& iter) { 
	
	// Iterate over real edges first, then switch to live edges if any

	if (iter.m_isLiveEdges)
	{
		// No more live edges, end iteration

		return iter.m_curr != iter.m_pEdges->end();
	}
	else
	{
		if (iter.m_currIndex == iter.m_pEdgeIndices->end())
		{
			iter.m_isLiveEdges = true;
			iter.m_pEdges = liveEdges(iter.m_nodeIndex);
			
			if (iter.m_pEdges == NULL || iter.m_pEdges->size() == 0)
			{
				// No live edges, end iteration

				return false;
			}
			else
			{
				// Switch to live edges

				iter.m_curr = iter.m_pEdges->begin();
				return true;
			}
		}
		else
		{
			// More real edges to iterate.

			return true;
		}
	}
}

template<typename Graph, typename EdgesMap>
void LiveGraphView<Graph, EdgesMap>::nextEdge(EdgeIterator& iter) 
{ 
	if (iter.m_isLiveEdges)
		++iter.m_curr; 
	else
		++iter.m_currIndex; 
}

template<typename Graph, typename EdgesMap>
typename LiveGraphView<Graph, EdgesMap>::LiveNodeIterator LiveGraphView<Graph, EdgesMap>::liveNodeIterator() const 
{ 
	return LiveNodeIterator(LiveStartNodeIndex); 
}

template<typename Graph, typename EdgesMap>
typename const Graph::Node* LiveGraphView<Graph, EdgesMap>::getLiveNode(LiveNodeIterator& iter) const 
{ 
	switch (iter.m_nodeIndex)
	{
		case LiveStartNodeIndex: return &m_startNode;
		case LiveGoalNodeIndex: return &m_goalNode;
	}

	return NULL;
}

template<typename Graph, typename EdgesMap>
typename Graph::Index LiveGraphView<Graph, EdgesMap>::getLiveNodeIndex(LiveNodeIterator& iter) const 
{
	switch (iter.m_nodeIndex)
	{
		case LiveStartNodeIndex: return LiveStartNodeIndex;
		case LiveGoalNodeIndex: return LiveGoalNodeIndex;
	}

	return NULL;
}

template<typename Graph, typename EdgesMap>
void LiveGraphView<Graph, EdgesMap>::nextLiveNode(LiveNodeIterator& iter) const 
{
	const Index nextOffset = LiveGoalNodeIndex > LiveStartNodeIndex ? 1 : -1;

	iter.m_nodeIndex += nextOffset;
}


template<typename Graph, typename EdgesMap>
typename Graph::Node& LiveGraphView<Graph, EdgesMap>::liveStartNode() 
{ 
	return m_startNode; 
}

template<typename Graph, typename EdgesMap>
typename Graph::Node& LiveGraphView<Graph, EdgesMap>::liveGoalNode() 
{ 
	return m_goalNode; 
}

template<typename Graph, typename EdgesMap>
typename Graph::Edges* LiveGraphView<Graph, EdgesMap>::getLiveEdges(const Index& nodeIndex, bool createIfNotFound) 
{ 
	Edges* pRetEdges = liveEdges(nodeIndex);

	if (pRetEdges == NULL && createIfNotFound)
	{
		if (nodeIndex != LiveGoalNodeIndex)
		{
			// Create edges that target the goal node.

			static const Edges empty;

			m_liveEdges[nodeIndex] = empty;
			pRetEdges = &m_liveEdges[nodeIndex];
		} 
		else
		{
			// edges starting at live goal are not allowed, 
			// to support this modifications in clearing live edges have to be made
			// this might cost some performance
		}
	}
	
	return pRetEdges;
}

template<typename Graph, typename EdgesMap>
void LiveGraphView<Graph, EdgesMap>::clearLiveEdges() 
{
	m_liveEdges.clear();
}

template<typename Graph, typename EdgesMap>
void LiveGraphView<Graph, EdgesMap>::clearStartLiveEdges() 
{
	EdgesMap::iterator found;
	if ((found = m_liveEdges.find(LiveStartNodeIndex)) != m_liveEdges.end())
	{
		m_liveEdges.erase(found);
	}
}

template<typename Graph, typename EdgesMap>
void LiveGraphView<Graph, EdgesMap>::clearGoalLiveEdges() 
{
	// Save start node edges, clear the whole map 
	// and then restore the start node edges.

	Edges* pStartEdges = getLiveEdges(LiveStartNodeIndex, false);
	Edges startEdges;

	if (pStartEdges)
	{
		startEdges = *pStartEdges;
	}

	m_liveEdges.clear();

	if (pStartEdges)
	{
		m_liveEdges[LiveStartNodeIndex] = startEdges;
	}
}

template<typename Graph, typename EdgesMap>
typename const Graph::EdgeIndices& LiveGraphView<Graph, EdgesMap>::edgeIndices(Graph& graph, const Index& nodeIndex) 
{ 
	return graph.getEdgeIndices(nodeIndex); 
}

template<typename Graph, typename EdgesMap>
typename Graph::Edges* LiveGraphView<Graph, EdgesMap>::liveEdges(const Index& nodeIndex) 
{ 
	EdgesMap::iterator found;
	if ((found = m_liveEdges.find(nodeIndex)) != m_liveEdges.end())
	{
		return &(found->second);
	}
	
	return NULL;
}

template<typename Graph, typename EdgesMap>
LiveGraphView<Graph, EdgesMap>::EdgeIterator::EdgeIterator(const Edges* pEdges, const Index& nodeIndex) 
:	m_nodeIndex(nodeIndex)
,	m_pEdges(pEdges) 
,	m_isLiveEdges(true)
{
	if (m_pEdges)
		m_curr = m_pEdges->begin();
}

template<typename Graph, typename EdgesMap>
LiveGraphView<Graph, EdgesMap>::EdgeIterator::EdgeIterator(const EdgeIndices* pEdgeIndices, const Index& nodeIndex, bool dummy) 
:	m_nodeIndex(nodeIndex)
,	m_pEdgeIndices(pEdgeIndices) 
,	m_isLiveEdges(false)
{
	if (m_pEdgeIndices)
		m_currIndex = m_pEdgeIndices->begin();
}

template<typename Graph, typename EdgesMap>
LiveGraphView<Graph, EdgesMap>::LiveNodeIterator::LiveNodeIterator(Index nodeIndex) 
:	m_nodeIndex(nodeIndex)
{
}


template <typename G, typename CV, typename GP>
NodeStateManager<G, CV, GP>::State::State() 
:	nodeIndex(InvalidNodeIndex)
,	f(0)
,	g(0)
,	h(0)
,	pathPredecessor(InvalidIndex)
,	status(NodeIsUnvisited)
{
}

template <typename G, typename CV, typename GP> template <typename PCE>
NodeStateManager<G, CV, GP>::OpenList_HeapCompare_f<PCE>::OpenList_HeapCompare_f(NodeStateManager& stateContainer, PCE& pathCostEval) 
:	m_stateManager(stateContainer)
,	m_pathCostEval(pathCostEval) 
{
}

template <typename G, typename CV, typename GP> template <typename PCE>
bool NodeStateManager<G, CV, GP>::OpenList_HeapCompare_f<PCE>::operator() (const Index& ref, const Index& comp) const
{
	const State& stateRef = m_stateManager.state(ref);
	const State& stateComp = m_stateManager.state(comp);

	if (PCE::CanBreakTies)
	{
		if (stateRef.f == stateComp.f)
			return m_pathCostEval.breakTie(m_stateManager, ref, comp);
	}

	return stateRef.f > stateComp.f;
}

template <typename G, typename CV, typename GP>
NodeStateManager<G, CV, GP>::NodeStateManager()
:	m_usedNodeSearchStateCount(0)
,	m_closedListSize(0)
{
}

template <typename G, typename CV, typename GP>
NodeStateManager<G, CV, GP>::~NodeStateManager()
{
}

template <typename G, typename CV, typename GP>
void NodeStateManager<G, CV, GP>::initSearch(size_t expectedVisitedNodeCount)
{
	m_openHeap.clear();
	m_stateIndexByNodeIndexMap.clear();
	m_usedNodeSearchStateCount = 0;
	m_closedListSize = 0;
	m_nodeSearchStates.resize(expectedVisitedNodeCount);
}

template <typename G, typename CV, typename GP>
NodeStateManager<G, CV, GP>& NodeStateManager<G, CV, GP>::operator=(const NodeStateManager& ref) 
{
	this->m_usedNodeSearchStateCount = ref.m_usedNodeSearchStateCount;
	this->m_closedListSize = ref.m_closedListSize;
	this->m_nodeSearchStates = States(ref.m_nodeSearchStates.begin(), ref.m_nodeSearchStates.begin() + ref.m_usedNodeSearchStateCount);
	this->m_openHeap = ref.m_openHeap;
	this->m_stateIndexByNodeIndexMap = ref.m_stateIndexByNodeIndexMap;

	return *this;
}

template <typename G, typename CV, typename GP>
typename NodeStateManager<G, CV, GP>::Index NodeStateManager<G, CV, GP>::newState()
{
	if (++m_usedNodeSearchStateCount > m_nodeSearchStates.size()) 
	{
		m_nodeSearchStates.resize(GP::grow(m_nodeSearchStates.size(), m_usedNodeSearchStateCount));
	}

	return m_usedNodeSearchStateCount - 1;
}

template <typename G, typename CV, typename GP> template <typename PCE>
void NodeStateManager<G, CV, GP>::reinsertOpenState(PCE& pathCostEval, Index stateIndex, Index newPathPredecessor, CostValue newG) 
{
	State& updatedState = state(stateIndex);

	updatedState.g = newG;
	updatedState.f = updatedState.g + updatedState.h;
	updatedState.pathPredecessor = newPathPredecessor;

	m_openHeap.push_back(stateIndex);
	std::push_heap(m_openHeap.begin(), m_openHeap.end(), OpenList_HeapCompare_f<PCE>(*this, pathCostEval));
}

template <typename G, typename CV, typename GP>
typename NodeStateManager<G, CV, GP>::OpenHeap::iterator NodeStateManager<G, CV, GP>::findInOpenList(GraphIndex nodeIndex)
{
	for (OpenHeap::iterator i = m_openHeap.begin(); i != m_openHeap.end(); ++i) 
	{
		if (state(*i).nodeIndex == nodeIndex) 
		{
			return i;
		}
	}

	return m_openHeap.end();
}

template <typename G, typename CV, typename GP> template <typename PCE>
void NodeStateManager<G, CV, GP>::openUnvisited(PCE& pathCostEval, GraphIndex nodeIndex, Index pathPredecessor, CostValue g, CostValue h) 
{
	Index stateIndex = newState();

	State& newState = state(stateIndex);

	newState.h = h;
	newState.nodeIndex = nodeIndex;
	newState.status = NodeIsOpen;
	m_stateIndexByNodeIndexMap[nodeIndex] = stateIndex;

	reinsertOpenState(pathCostEval, stateIndex, pathPredecessor, g);
}

template <typename G, typename CV, typename GP> template <typename PCE>
void NodeStateManager<G, CV, GP>::updateOpen(PCE& pathCostEval, ListIterator& openListIter, Index newPathPredecessor, CostValue newG) 
{
	Index stateIdx = stateIndex(openListIter);

	m_openHeap.erase(findInOpenList(nodeIndex(state(stateIdx))));
	std::make_heap(m_openHeap.begin(), m_openHeap.end(), OpenList_HeapCompare_f<PCE>(*this, pathCostEval));

	reinsertOpenState(pathCostEval, stateIdx, newPathPredecessor, newG);
}

template <typename G, typename CV, typename GP> template <typename PCE> 
void NodeStateManager<G, CV, GP>::openClosed(PCE& pathCostEval, ListIterator& closedListIter, Index newPathPredecessor, CostValue newG) 
{
	--m_closedListSize;
	Index stateIdx =  stateIndex(closedListIter);
	GraphIndex closedNodeIndex = nodeIndex(stateIdx);

	state(stateIdx).status = NodeIsOpen;

	reinsertOpenState(pathCostEval, stateIdx, newPathPredecessor, newG);
}

template <typename G, typename CV, typename GP> template <typename PCE>
void NodeStateManager<G, CV, GP>::closeBestOpen(PCE& pathCostEval) 
{
	++m_closedListSize;
	Index stateIndex = m_openHeap.front();
	GraphIndex openNodeIndex = nodeIndex(stateIndex);

	state(stateIndex).status = NodeIsClosed;

	std::pop_heap(m_openHeap.begin(), m_openHeap.end(), OpenList_HeapCompare_f<PCE>(*this, pathCostEval));
	m_openHeap.pop_back();
}

template <typename G, typename CV, typename GP>
NodeStatus NodeStateManager<G, CV, GP>::getNodeStatus(const GraphIndex& nodeIndex, ListIterator& result) const
{
	{
		IndexByNodeIndexMap::const_iterator visitedIt = m_stateIndexByNodeIndexMap.find(nodeIndex);
		if (visitedIt != m_stateIndexByNodeIndexMap.end())
		{
			result.iter = visitedIt;
			return state(stateIndex(result)).status;
		}
	}

	return NodeIsUnvisited;
}

template <typename G, typename CV, typename GP>
void NodeStateManager<G, CV, GP>::extractOpenList(vector<GraphIndex>& nodes) const
{
	nodes.clear();
	nodes.reserve(openListSize());

	for (OpenHeap::const_iterator it = m_openHeap.begin(); it != m_openHeap.end(); ++it) {

		nodes.push_back(nodeIndex(*it));
	}
}

template <typename G, typename CV, typename GP>
void NodeStateManager<G, CV, GP>:: extractClosedList(vector<GraphIndex>& nodes) const
{
	nodes.clear();
	nodes.reserve(closedListSize());

	for (Index i = 0; i < m_usedNodeSearchStateCount; ++i) {

		if (state(i).status == NodeIsClosed)
			nodes.push_back(nodeIndex(state(i)));
	}
}

#endif