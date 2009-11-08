/******************************************************************************
 * This file is part of The AI Sandbox.
 * 
 * Copyright (c) 2008, AiGameDev.com
 * 
 * Credits:         See the PEOPLE file in the base directory.
 * License:         This software may be used for your own personal research
 *                  and education only.  For details, see the LICENSING file.
 *****************************************************************************/

#ifndef GRAPH_H
#define GRAPH_H

/// @file           Graph.h
/// @brief          A template-based directed graph structure that manages the
///         connections for the nodes.

//#include <core/Base.h>
//#include <core/Assert.h>
#include <vector>

template <class NODE, class EDGE>
class Graph
{
public:
    typedef int Index;
	typedef typename std::vector<NODE> Nodes;
    typedef typename std::vector<EDGE> Edges;
	typedef typename std::vector<Index> EdgeIndices;
	typedef NODE Node;
	typedef EDGE Edge;

	enum { InvalidIndex = (Index) -1 };

	Graph();
	explicit Graph(const Graph& ref);

	void clear();

    Index addNode(const NODE& node);
    NODE& getNode(Index index);
	const NODE& getNode(Index index) const;
    void removeNode(Index index);

    Index connect(Index index, const EDGE& edge);
    EdgeIndices& getEdgeIndices(Index index);
	const EdgeIndices& getEdgeIndices(Index index) const;
	NODE& getNode(const EdgeIndices::const_iterator& edgeIt);
	const NODE& getNode(const EdgeIndices::const_iterator& edgeIt) const;
	EDGE& getEdge(Index edgeIndex);
	const EDGE& getEdge(Index edgeIndex) const;
	EDGE& getNodeEdge(Index nodeIndex, Index edgeIndex);
    EDGE* getEdge(Index from, Index to);
	Index getEdgeIndex(Index from, Index to);
    void disconnect(Index index);

    Index getSize() const;
	Index getEdgeCount() const;

	EdgeIndices::iterator findEdgeTo(EdgeIndices& edgeIndices, Index targetNodeIndex);
	void eraseEdge(EdgeIndices& edgeIndices, EdgeIndices::iterator& edgeIndexIter);

	void removeEdges(Index previousTarget);
	void repointEdges(Index previousTarget, Index newTarget);

	Index indexOf(const Node& node) const;
	Index indexOf(const Edge& edge) const;

protected:

    Nodes m_Nodes;
	Edges m_Edges;
	std::vector<EdgeIndices> m_NodeEdges;
};


struct Edge
{
    Edge(int n = -1)
    :    neighbor(n)
    {
    }

	bool operator==(const int other) const { return neighbor == other; }
	bool operator==(const Edge& other) const { return neighbor == other.neighbor; }

	operator int() const { return neighbor; }

    int neighbor;
};


template <class NODE, class EDGE>
Graph<NODE, EDGE>::Graph()
{
}

template <class NODE, class EDGE>
Graph<NODE, EDGE>::Graph(const Graph& ref)
:	m_Nodes(ref.m_Nodes)
,	m_Edges(ref.m_Edges)
,	m_NodeEdges(ref.m_NodeEdges)
{
}

template <class NODE, class EDGE>
void Graph<NODE, EDGE>::clear()
{
	m_Nodes.clear();
	m_Edges.clear();
	m_NodeEdges.clear();
}

template <class NODE, class EDGE>
typename Graph<NODE, EDGE>::EdgeIndices& Graph<NODE, EDGE>::getEdgeIndices(int index)
{
	return m_NodeEdges[index];
}

template <class NODE, class EDGE>
typename const Graph<NODE, EDGE>::EdgeIndices& Graph<NODE, EDGE>::getEdgeIndices(int index) const
{
	return m_NodeEdges[index];
}

template <class NODE, class EDGE>
EDGE& Graph<NODE, EDGE>::getEdge(Index edgeIndex)
{
	return m_Edges[edgeIndex];
}

template <class NODE, class EDGE>
EDGE& Graph<NODE, EDGE>::getNodeEdge(Index nodeIndex, Index edgeIndex)
{
	return m_Edges[m_NodeEdges[nodeIndex][edgeIndex]];
}

template <class NODE, class EDGE>
const EDGE& Graph<NODE, EDGE>::getEdge(Index edgeIndex) const
{
	return m_Edges[edgeIndex];
}


template <class NODE, class EDGE>
EDGE* Graph<NODE, EDGE>::getEdge(Index from, Index to)
{
	EdgeIndices& edgeIndices = m_NodeEdges[from];
	EdgeIndices::iterator& it = findEdgeTo(edgeIndices, to);

	return it == edgeIndices.end() ? NULL : &(m_Edges[*it]);
}

template <class NODE, class EDGE>
typename Graph<NODE, EDGE>::Index Graph<NODE, EDGE>::getEdgeIndex(Index from, Index to)
{
	EdgeIndices& edgeIndices = m_NodeEdges[from];
	EdgeIndices::iterator& it = findEdgeTo(edgeIndices, to);

	return it == edgeIndices.end() ? InvalidIndex : *it;
}

template <class NODE, class EDGE>
NODE& Graph<NODE, EDGE>::getNode(const EdgeIndices::const_iterator& edgeIt)
{
	return getNode(getEdge(*edgeIt));
}

template <class NODE, class EDGE>
const NODE& Graph<NODE, EDGE>::getNode(const EdgeIndices::const_iterator& edgeIt) const
{
	return getNode(getEdge(*edgeIt));
}

template <class NODE, class EDGE>
Graph<NODE, EDGE>::EdgeIndices::iterator Graph<NODE, EDGE>::findEdgeTo(EdgeIndices& edgeIndices, Index targetNodeIndex)
{
	EdgeIndices::iterator it = edgeIndices.begin();

	for (; it != edgeIndices.end(); ++it)
	{
		if ((getEdge(*it)) == targetNodeIndex)
			return it;
	}

	return it;
}

template <class NODE, class EDGE>
void Graph<NODE, EDGE>::eraseEdge(EdgeIndices& edgeIndices, EdgeIndices::iterator& edgeIndexIter)
{
	//OPTIMIZATION use a free list
	edgeIndices.erase(edgeIndexIter);
}

template <class NODE, class EDGE>
typename Graph<NODE, EDGE>::Index Graph<NODE, EDGE>::connect(int index, const EDGE& edge)
{
	ASSERT_MSG(terrain, index != (Index) edge, "Trying to connect node to itself");

	EdgeIndices& edgeIndices = m_NodeEdges[index];
	EdgeIndices::iterator it = findEdgeTo(edgeIndices, (Index) edge);
	if (it != edgeIndices.end())
	{
		return *it;
	}

	//OPTIMIZATION use a free list
	edgeIndices.push_back(m_Edges.size());
	m_Edges.push_back(edge);
	
	return m_Edges.size() - 1;
}


template <class NODE, class EDGE>
void Graph<NODE, EDGE>::disconnect(Index index)
{
	m_NodeEdges[index].clear();
}

template <class NODE, class EDGE>
void Graph<NODE, EDGE>::repointEdges(Index previousTarget, Index newTarget)
{
	std::replace(m_Edges.begin(), m_Edges.end(), previousTarget, newTarget);
}


template <class NODE, class EDGE>
void Graph<NODE, EDGE>::removeEdges(Index previousTarget)
{
	for (int i=0; i<(int)m_NodeEdges.size(); ++i)
	{
		EdgeIndices& indices = m_NodeEdges[i];
		for (EdgeIndices::iterator it = indices.begin(); it != indices.end(); )
		{
			EDGE& e = m_Edges[*it];
			if (e == previousTarget)
			{
				it = indices.erase(it);
				continue;
			}
			++it;
		}
	}
}

template <class NODE, class EDGE>
void Graph<NODE, EDGE>::removeNode(int index)
{
	m_Nodes[index] = m_Nodes.back();
	m_NodeEdges[index] = m_NodeEdges.back();

	int previous = (int)m_Nodes.size() - 1;
	removeEdges(index);
	repointEdges(previous, index);
	
	m_NodeEdges.pop_back();
	m_Nodes.pop_back();
}


template <class NODE, class EDGE>
NODE& Graph<NODE, EDGE>::getNode(int index)
{
	return m_Nodes[index];
}

template <class NODE, class EDGE>
const NODE& Graph<NODE, EDGE>::getNode(int index) const
{
	return m_Nodes[index];
}

template <class NODE, class EDGE>
typename Graph<NODE, EDGE>::Index Graph<NODE, EDGE>::getSize() const
{
	return m_Nodes.size();
}

template <class NODE, class EDGE>
typename Graph<NODE, EDGE>::Index Graph<NODE, EDGE>::getEdgeCount() const
{
	return m_Edges.size();
}


template <class NODE, class EDGE>
typename Graph<NODE, EDGE>::Index Graph<NODE, EDGE>::addNode(const NODE& node)
{
	m_Nodes.push_back(node);
	m_NodeEdges.resize(m_Nodes.size());
	return m_Nodes.size() - 1;
}

//NOTE this is not clean, remove the need for this function in the whole sandbox
template <class NODE, class EDGE>
typename Graph<NODE, EDGE>::Index Graph<NODE, EDGE>::indexOf(const Node& node) const 
{ 
	return (&node)- &(m_Nodes[0]); 
}

//NOTE this is not clean, remove the need for this function in the whole sandbox
template <class NODE, class EDGE>
typename Graph<NODE, EDGE>::Index Graph<NODE, EDGE>::indexOf(const Edge& edge) const 
{ 
	return (&edge)- &(m_Edges[0]); 
}

#endif // TERRAIN_GRAPH_H
