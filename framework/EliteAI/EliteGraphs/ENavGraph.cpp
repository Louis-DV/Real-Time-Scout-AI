#include "stdafx.h"
#include "ENavGraph.h"
#include "framework\EliteAI\EliteGraphs\EliteGraphAlgorithms\EAStar.h"

using namespace Elite;

Elite::NavGraph::NavGraph(const Polygon& contourMesh, float playerRadius = 1.0f) :
	Graph2D(false),
	m_pNavMeshPolygon(nullptr)
{
	//Create the navigation mesh (polygon of navigatable area= Contour - Static Shapes)
	m_pNavMeshPolygon = new Polygon(contourMesh); // Create copy on heap

	//Get all shapes from all static rigidbodies with NavigationCollider flag
	auto vShapes = PHYSICSWORLD->GetAllStaticShapesInWorld(PhysicsFlags::NavigationCollider);

	//Store all children
	for (auto shape : vShapes)
	{
		shape.ExpandShape(playerRadius);
		m_pNavMeshPolygon->AddChild(shape);
	}

	//Triangulate
	m_pNavMeshPolygon->Triangulate();

	//Create the actual graph (nodes & connections) from the navigation mesh
	CreateNavigationGraph();
}

Elite::NavGraph::~NavGraph()
{
	delete m_pNavMeshPolygon; 
	m_pNavMeshPolygon = nullptr;
}

int Elite::NavGraph::GetNodeIdxFromLineIdx(int lineIdx) const
{
	auto nodeIt = std::find_if(m_Nodes.begin(), m_Nodes.end(), [lineIdx](const NavGraphNode* n) { return n->GetLineIndex() == lineIdx; });
	if (nodeIt != m_Nodes.end())
	{
		return (*nodeIt)->GetIndex();
	}

	return invalid_node_index;
}

Elite::Polygon* Elite::NavGraph::GetNavMeshPolygon() const
{
	return m_pNavMeshPolygon;
}

void Elite::NavGraph::CreateNavigationGraph()
{
	//1. Go over all the edges of the navigationmesh and create nodes
	for (Line* line : m_pNavMeshPolygon->GetLines())
	{
		if (m_pNavMeshPolygon->GetTrianglesFromLineIndex(line->index).size() > 1)
		{
			AddNode(new NavGraphNode(GetNextFreeNodeIndex(), line->index, (line->p1 + line->p2) / 2));
		}
	}
	//2. Create connections now that every node is created
	
	std::vector<int> indexes;

	for (Triangle* triangle : m_pNavMeshPolygon->GetTriangles())
	{
		for (int index : triangle->metaData.IndexLines)
		{
			for (NavGraphNode* node : GetAllNodes())
			{
				if (node->GetLineIndex() == index)
				{
					indexes.push_back(GetNodeIdxFromLineIdx(index));
				}
			}
		}
		if (indexes.size() == 2)
		{
			AddConnection(new Elite::GraphConnection2D(indexes[0], indexes[1]));
		}
		else if(indexes.size() == 3)
		{
			AddConnection(new Elite::GraphConnection2D(indexes[0], indexes[1]));
			AddConnection(new Elite::GraphConnection2D(indexes[1], indexes[2]));
			AddConnection(new Elite::GraphConnection2D(indexes[2], indexes[0]));
		}
		indexes.clear();
	}

	//3. Set the connections cost to the actual distance
	SetConnectionCostsToDistance();
}

