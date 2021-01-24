//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

//Includes
#include "App_ResearchProject.h"
#include "framework\EliteAI\EliteGraphs\EliteGraphAlgorithms\EAstar.h"
#include "framework\EliteAI\EliteGraphs\EliteGraphAlgorithms\EBFS.h"
#include <algorithm>

using namespace Elite;

//Destructor
App_ResearchProject::~App_ResearchProject()
{
	SAFE_DELETE(m_pGridGraph);
	SAFE_DELETE(m_pMappedGridGraph);
	SAFE_DELETE(m_pSeekBehavior);
	SAFE_DELETE(m_pAgent);
}

//Functions
void App_ResearchProject::Start()
{
	//Set Camera
	DEBUGRENDERER2D->GetActiveCamera()->SetZoom(39.0f);
	DEBUGRENDERER2D->GetActiveCamera()->SetCenter(Elite::Vector2(73.0f, 35.0f));
	//DEBUGRENDERER2D->GetActiveCamera()->SetMoveLocked(true);
	//DEBUGRENDERER2D->GetActiveCamera()->SetZoomLocked(true);

	//Create Graph
	MakeGridGraph();

	startPathIdx = 0;
	endPathIdx = 101;

	m_pSeekBehavior = new Seek();
	m_pAgent = new SteeringAgent();
	m_pAgent->SetSteeringBehavior(m_pSeekBehavior);
	m_pAgent->SetMaxLinearSpeed(m_AgentSpeed);
	m_pAgent->SetAutoOrient(true);
	m_pAgent->SetMass(.1f);
	m_pAgent->SetPosition({ 15,55 });

}

void App_ResearchProject::Update(float deltaTime)
{
	UNREFERENCED_PARAMETER(deltaTime);

	//INPUT
	bool const middleMousePressed = INPUTMANAGER->IsMouseButtonUp(InputMouseButton::eMiddle);
	if (middleMousePressed)
	{
		MouseData mouseData = { INPUTMANAGER->GetMouseData(Elite::InputType::eMouseButton, Elite::InputMouseButton::eMiddle) };
		Elite::Vector2 mousePos = DEBUGRENDERER2D->GetActiveCamera()->ConvertScreenToWorld({ (float)mouseData.X, (float)mouseData.Y });
		//Find closest node to click pos
		int closestNode = m_pMappedGridGraph->GetNodeFromWorldPos(mousePos);
		if (m_StartSelected)
		{
			startPathIdx = closestNode;
			m_UpdatePath = true;
		}
		else
		{
			endPathIdx = closestNode;
			m_UpdatePath = true;
		}
	}
	
	int newStart = m_pMappedGridGraph->GetNodeFromWorldPos(m_pAgent->GetPosition());
	if (newStart != startPathIdx)
	{
		startPathIdx = newStart;
		m_UpdatePath = true;
	}

	//GRID INPUT
	bool hasGridChanged = m_GraphEditor.UpdateGraph(m_pGridGraph);
	if (hasGridChanged)
	{
		m_UpdatePath = true;
	}

	//IMGUI
	UpdateImGui();


	//Change sight radius according to the current node, and set scores if on forest or hill  
	GridTerrainNode* pCurrentNode = m_pMappedGridGraph->GetNode(m_pMappedGridGraph->GetNodeFromWorldPos(m_pAgent->GetPosition()));

	if (pCurrentNode->GetTerrainType() == TerrainType::Hill)
	{
		m_SightRadius = 35.f;
		if (DistanceSquared(m_pAgent->GetPosition(), m_pMappedGridGraph->GetNodeWorldPos(pCurrentNode)) < 1 && m_SeenNodesMap.count(pCurrentNode->GetIndex()) == 0)
		{
			SetHillScore();
		}
	}
	else if (pCurrentNode->GetTerrainType() == TerrainType::Forest)
	{
		m_SightRadius = 15.f;
		if (DistanceSquared(m_pAgent->GetPosition(), m_pMappedGridGraph->GetNodeWorldPos(pCurrentNode)) < 1 && m_SeenNodesMap.count(pCurrentNode->GetIndex()) == 0)
		{
			SetForestScore();
		}
	}
	else
	{
		m_SightRadius = 25.f;
	}



	//Put all visible nodes in map graph
	for (GridTerrainNode* node : m_pMappedGridGraph->GetAllNodes())
	{
		if (DistanceSquared(m_pMappedGridGraph->GetNodeWorldPos(node->GetIndex()), m_pAgent->GetPosition()) <= m_SightRadius * m_SightRadius)
		{


			node->SetTerrainType(m_pGridGraph->GetNode(node->GetIndex())->GetTerrainType());
			m_pMappedGridGraph->UnIsolateNode(node->GetIndex());


		}
	}

	//Go over all visible nodes without a score and set the score if possible
	for (GridTerrainNode* node : m_pMappedGridGraph->GetAllNodes())
	{
		if (DistanceSquared(m_pMappedGridGraph->GetNodeWorldPos(node->GetIndex()),m_pAgent->GetPosition()) <= m_SightRadius*m_SightRadius)
		{

			node->SetTerrainType(m_pGridGraph->GetNode(node->GetIndex())->GetTerrainType());
			m_pMappedGridGraph->UnIsolateNode(node->GetIndex());

			bool important{};
			if (node->GetTerrainType() == TerrainType::Ground)
			{
				for (GraphConnection* connection : m_pMappedGridGraph->GetConnections(node->GetIndex()))
				{
					if (m_pMappedGridGraph->GetNode(connection->GetTo())->GetTerrainType() != TerrainType::Ground)
						important = true;
				}
			}

			if (node->GetTerrainType() == TerrainType::Hill || node->GetTerrainType() == TerrainType::Forest)
			{
				important = true;
			}

			if (m_SeenNodesMap.count(node->GetIndex()) == 1)
			{
				important = false;
			}
			else
			{
				if (SetNodeScore(node->GetIndex()))
				{
					important = false;
				}
				else
				{
					important = true;
				}
			}
			auto it = std::find(m_ImportantNodeList.begin(), m_ImportantNodeList.end(), node->GetIndex());
			if (it != m_ImportantNodeList.end())
			{
				if (!important)
				{
					m_ImportantNodeList.erase(it);
				}
			}
			else
			{
				if (important)
				{
					m_ImportantNodeList.push_back(node->GetIndex());
				}
			}
		}
	}

	//Set endPathIdx to closest important node, or closest hill if any hills are in the list
	if (!m_ImportantNodeList.empty())
	{
		int newEndPathIdx = *std::min_element(m_ImportantNodeList.begin(), m_ImportantNodeList.end(), [this](int idx1, int idx2) 
			{
			
				if ((m_pMappedGridGraph->GetNode(idx1)->GetTerrainType() == TerrainType::Hill && m_pMappedGridGraph->GetNode(idx2)->GetTerrainType() == TerrainType::Hill) || (m_pMappedGridGraph->GetNode(idx1)->GetTerrainType() != TerrainType::Hill && m_pMappedGridGraph->GetNode(idx2)->GetTerrainType() != TerrainType::Hill))
				{
					return DistanceSquared(m_pAgent->GetPosition(), m_pMappedGridGraph->GetNodeWorldPos(idx1)) < DistanceSquared(m_pAgent->GetPosition(), m_pMappedGridGraph->GetNodeWorldPos(idx2));
				}
				else
				{
					return m_pMappedGridGraph->GetNode(idx1)->GetTerrainType() == TerrainType::Hill;
				}
			
			});

		if (newEndPathIdx != endPathIdx)
		{
			m_UpdatePath = true;
			endPathIdx = newEndPathIdx;
		}
	}
	
	//CALCULATEPATH
	//If we have nodes and the target is not the startNode, find a path!
	if (m_UpdatePath 
		&& startPathIdx != invalid_node_index
		&& endPathIdx != invalid_node_index
		&& startPathIdx != endPathIdx)
	{
		//BFS Pathfinding
		auto pathfinder = AStar<GridTerrainNode, GraphConnection>(m_pMappedGridGraph, m_pHeuristicFunction);
		auto startNode = m_pMappedGridGraph->GetNode(startPathIdx);
		auto endNode = m_pMappedGridGraph->GetNode(endPathIdx);

		m_vPath = pathfinder.FindPath(startNode, endNode);

		m_UpdatePath = false;
		std::cout << "New Path Calculated" << std::endl;

		m_pSeekBehavior->SetTarget(TargetData(m_pGridGraph->GetNodeWorldPos(m_vPath[1])));
	}


	m_pAgent->Update(deltaTime);
}

void App_ResearchProject::Render(float deltaTime) const
{
	UNREFERENCED_PARAMETER(deltaTime);

	m_GraphRenderer.RenderGraph(
		m_pMappedGridGraph, 
		m_bDrawGrid, 
		m_bDrawNodeNumbers, 
		m_bDrawConnections, 
		m_bDrawConnectionsCosts
	);

	for (pair<int,int> pair : m_SeenNodesMap)
	{
		DEBUGRENDERER2D->DrawString(m_pMappedGridGraph->GetNodeWorldPos(pair.first), std::to_string(pair.second).c_str());
	}
	
	m_pAgent->Render(deltaTime);

	DEBUGRENDERER2D->DrawCircle(m_pAgent->GetPosition(), m_SightRadius, { 1.f,0.f,0.f }, 0.1f);
}

void App_ResearchProject::MakeGridGraph()
{
	m_pGridGraph = new GridGraph<GridTerrainNode, GraphConnection>(COLUMNS, ROWS, m_SizeCell, false, false, 1.f, 1.5f);
	m_pMappedGridGraph = new GridGraph<GridTerrainNode, GraphConnection>(COLUMNS, ROWS, m_SizeCell, false, false, 1.f, 1.5f);
	for (GridTerrainNode* node : m_pMappedGridGraph->GetAllNodes())
	{
		node->SetTerrainType(TerrainType::Unknown);
		m_pMappedGridGraph->UnIsolateNode(node->GetIndex());
	}

	CreateMap();

}

void App_ResearchProject::UpdateImGui()
{
#ifdef PLATFORM_WINDOWS
#pragma region UI
	//UI
	{
		//Setup
		int menuWidth = 115;
		int const width = DEBUGRENDERER2D->GetActiveCamera()->GetWidth();
		int const height = DEBUGRENDERER2D->GetActiveCamera()->GetHeight();
		bool windowActive = true;
		ImGui::SetNextWindowPos(ImVec2((float)width - menuWidth - 10, 10));
		ImGui::SetNextWindowSize(ImVec2((float)menuWidth, (float)height - 20));
		ImGui::Begin("Gameplay Programming", &windowActive, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
		ImGui::PushAllowKeyboardFocus(false);

		//Elements
		ImGui::Text("CONTROLS");
		ImGui::Indent();
		ImGui::Text("LMB: target");
		ImGui::Text("RMB: start");
		ImGui::Unindent();

		/*Spacing*/ImGui::Spacing();ImGui::Separator();ImGui::Spacing();ImGui::Spacing();

		ImGui::Text("STATS");
		ImGui::Indent();
		ImGui::Text("%.3f ms/frame", 1000.0f / ImGui::GetIO().Framerate);
		ImGui::Text("%.1f FPS", ImGui::GetIO().Framerate);
		ImGui::Unindent();

		/*Spacing*/ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing(); ImGui::Spacing();

		ImGui::Text("A* Pathfinding");
		ImGui::Spacing();

		ImGui::Text("Middle Mouse");
		ImGui::Text("controls");
		std::string buttonText{""};
		if (m_StartSelected)
			buttonText += "Start Node";
		else
			buttonText += "End Node";

		if (ImGui::Button(buttonText.c_str()))
		{
			m_StartSelected = !m_StartSelected;
		}

		ImGui::Checkbox("Grid", &m_bDrawGrid);
		ImGui::Checkbox("NodeNumbers", &m_bDrawNodeNumbers);
		ImGui::Checkbox("Connections", &m_bDrawConnections);
		ImGui::Checkbox("Connections Costs", &m_bDrawConnectionsCosts);
		if (ImGui::Combo("", &m_SelectedHeuristic, "Manhattan\0Euclidean\0SqrtEuclidean\0Octile\0Chebyshev", 4))
		{
			switch (m_SelectedHeuristic)
			{
			case 0:
				m_pHeuristicFunction = HeuristicFunctions::Manhattan;
				break;
			case 1:
				m_pHeuristicFunction = HeuristicFunctions::Euclidean;
				break;
			case 2:
				m_pHeuristicFunction = HeuristicFunctions::SqrtEuclidean;
				break;
			case 3:
				m_pHeuristicFunction = HeuristicFunctions::Octile;
				break;
			case 4:
				m_pHeuristicFunction = HeuristicFunctions::Chebyshev;
				break;
			default:
				m_pHeuristicFunction = HeuristicFunctions::Chebyshev;
				break;
			}
		}
		ImGui::Spacing();

		//End
		ImGui::PopAllowKeyboardFocus();
		ImGui::End();
	}
#pragma endregion
#endif
}

void App_ResearchProject::SetHillScore()
{
	int score = 2;

	for (GridTerrainNode* node : m_pMappedGridGraph->GetAllNodes())
	{
		if (DistanceSquared(m_pMappedGridGraph->GetNodeWorldPos(node->GetIndex()), m_pAgent->GetPosition()) <= m_SightRadius * m_SightRadius)
		{
			if (node->GetTerrainType() != TerrainType::Ground)
			{
				score += 1;
			}
		}
	}

	if (score > 10)
		score = 10;

	m_SeenNodesMap[m_pGridGraph->GetNodeFromWorldPos(m_pAgent->GetPosition())] = score;
}

void App_ResearchProject::SetForestScore()
{
	int score = -6;

	for (GridTerrainNode* node : m_pMappedGridGraph->GetAllNodes())
	{
		if (DistanceSquared(m_pMappedGridGraph->GetNodeWorldPos(node->GetIndex()), m_pAgent->GetPosition()) <= m_SightRadius * m_SightRadius)
		{
			if (node->GetTerrainType() == TerrainType::Ground || node->GetTerrainType() == TerrainType::Hill || node->GetTerrainType() == TerrainType::Mud)
			{
				score = 6;
			}
		}
	}
	m_SeenNodesMap[m_pGridGraph->GetNodeFromWorldPos(m_pAgent->GetPosition())] = score;
}

bool App_ResearchProject::SetNodeScore(int idx)
{
	//Check terrain of the node itself
	int score = 0;
	if (m_pMappedGridGraph->GetNode(idx)->GetTerrainType() == TerrainType::Hill || m_pMappedGridGraph->GetNode(idx)->GetTerrainType() == TerrainType::Forest)
		return false;

	if (m_pMappedGridGraph->GetNode(idx)->GetTerrainType() == TerrainType::Water)
	{
		m_SeenNodesMap[idx] = -10;
		return true;
	}
	
	if (m_pMappedGridGraph->GetNode(idx)->GetTerrainType() == TerrainType::Mud)
	{
		score -= 5;
	}

	//get all 4 adjacent nodes and put in list
	int north = m_pMappedGridGraph->GetNodeFromWorldPos( m_pMappedGridGraph->GetNodeWorldPos(idx)  + Vector2{0, (float)m_SizeCell} );
	int east = m_pMappedGridGraph->GetNodeFromWorldPos( m_pMappedGridGraph->GetNodeWorldPos(idx)  + Vector2{ (float)m_SizeCell,0} );
	int south = m_pMappedGridGraph->GetNodeFromWorldPos( m_pMappedGridGraph->GetNodeWorldPos(idx)  + Vector2{0, -(float)m_SizeCell} );
	int west = m_pMappedGridGraph->GetNodeFromWorldPos( m_pMappedGridGraph->GetNodeWorldPos(idx)  + Vector2{-(float)m_SizeCell,0} );

	std::vector<int> connectionIndices{north,east,south,west};
	int badAdjacentNodeAmount{0};

	//Adjust score for adjacent forests and hills, and check if adjacent to unknown nodes, mud or water 
	for (int connection : connectionIndices)
	{
		if (connection != -1)
		{
			if (m_pMappedGridGraph->GetNode(connection)->GetTerrainType() == TerrainType::Unknown)
				return false;

			if (m_pMappedGridGraph->GetNode(connection)->GetTerrainType() == TerrainType::Forest)
			{
				score -= 3;
			}
			if (m_pMappedGridGraph->GetNode(connection)->GetTerrainType() == TerrainType::Hill)
			{
				score -= 3;
			}
			if (m_pMappedGridGraph->GetNode(connection)->GetTerrainType() == TerrainType::Mud || m_pMappedGridGraph->GetNode(connection)->GetTerrainType() == TerrainType::Water)
			{
				++badAdjacentNodeAmount;
			}
		}
	}

	//Adjust score for adjacent water and mud 
	if (badAdjacentNodeAmount >= 3)
	{
		score -= 7;
	}
	else if (badAdjacentNodeAmount == 2)
	{
		if ((north != -1 && south != -1) 
			&& (m_pMappedGridGraph->GetNode(north)->GetTerrainType() == TerrainType::Mud || m_pMappedGridGraph->GetNode(north)->GetTerrainType() == TerrainType::Water) 
			&& (m_pMappedGridGraph->GetNode(south)->GetTerrainType() == TerrainType::Mud || m_pMappedGridGraph->GetNode(south)->GetTerrainType() == TerrainType::Water))
		{
			score += 10;
		}
		else if ((east != -1 && west != -1)
			&& (m_pMappedGridGraph->GetNode(east)->GetTerrainType() == TerrainType::Mud || m_pMappedGridGraph->GetNode(east)->GetTerrainType() == TerrainType::Water)
			&& (m_pMappedGridGraph->GetNode(west)->GetTerrainType() == TerrainType::Mud || m_pMappedGridGraph->GetNode(west)->GetTerrainType() == TerrainType::Water))
		{
			score += 10;
		}
		else
		{
			score -= 5;
		}

	}
	else if (badAdjacentNodeAmount == 1)
	{
		if (east != -1 && (m_pMappedGridGraph->GetNode(east)->GetTerrainType() == TerrainType::Mud || m_pMappedGridGraph->GetNode(east)->GetTerrainType() == TerrainType::Water))
		{
			score += 3;
		}
		if (west != -1 && (m_pMappedGridGraph->GetNode(west)->GetTerrainType() == TerrainType::Mud || m_pMappedGridGraph->GetNode(west)->GetTerrainType() == TerrainType::Water))
		{
			score -= 3;
		}

	}

	if (score > 10)
	{
		score = 10;
	}
	else if (score < -10)
	{
		score = -10;
	}

	m_SeenNodesMap[idx] = score;

	return true;
}

void App_ResearchProject::CreateMap()
{

	m_pGridGraph->GetNode(6)->SetTerrainType(TerrainType::Mud);
	m_pGridGraph->GetNode(26)->SetTerrainType(TerrainType::Mud);
	m_pGridGraph->GetNode(5)->SetTerrainType(TerrainType::Mud);
	m_pGridGraph->GetNode(7)->SetTerrainType(TerrainType::Water);
	m_pGridGraph->GetNode(27)->SetTerrainType(TerrainType::Water);
	m_pGridGraph->GetNode(47)->SetTerrainType(TerrainType::Water);
	m_pGridGraph->GetNode(67)->SetTerrainType(TerrainType::Water);
	m_pGridGraph->GetNode(68)->SetTerrainType(TerrainType::Water);
	m_pGridGraph->GetNode(88)->SetTerrainType(TerrainType::Water);
	m_pGridGraph->GetNode(128)->SetTerrainType(TerrainType::Water);
	m_pGridGraph->GetNode(148)->SetTerrainType(TerrainType::Water);
	m_pGridGraph->GetNode(148)->SetTerrainType(TerrainType::Water);
	m_pGridGraph->GetNode(168)->SetTerrainType(TerrainType::Water);
	m_pGridGraph->GetNode(169)->SetTerrainType(TerrainType::Water);
	m_pGridGraph->GetNode(189)->SetTerrainType(TerrainType::Water);
	m_pGridGraph->GetNode(14)->SetTerrainType(TerrainType::Water);
	m_pGridGraph->GetNode(34)->SetTerrainType(TerrainType::Water);
	m_pGridGraph->GetNode(54)->SetTerrainType(TerrainType::Water);
	m_pGridGraph->GetNode(74)->SetTerrainType(TerrainType::Water);
	m_pGridGraph->GetNode(127)->SetTerrainType(TerrainType::Mud);
	m_pGridGraph->GetNode(126)->SetTerrainType(TerrainType::Mud);
	m_pGridGraph->GetNode(147)->SetTerrainType(TerrainType::Mud);
	m_pGridGraph->GetNode(187)->SetTerrainType(TerrainType::Mud);
	m_pGridGraph->GetNode(188)->SetTerrainType(TerrainType::Mud);
	m_pGridGraph->GetNode(87)->SetTerrainType(TerrainType::Mud);
	m_pGridGraph->GetNode(86)->SetTerrainType(TerrainType::Mud);
	m_pGridGraph->GetNode(66)->SetTerrainType(TerrainType::Mud);
	m_pGridGraph->GetNode(46)->SetTerrainType(TerrainType::Mud);
	m_pGridGraph->GetNode(8)->SetTerrainType(TerrainType::Forest);
	m_pGridGraph->GetNode(9)->SetTerrainType(TerrainType::Forest);
	m_pGridGraph->GetNode(10)->SetTerrainType(TerrainType::Forest);
	m_pGridGraph->GetNode(28)->SetTerrainType(TerrainType::Forest);
	m_pGridGraph->GetNode(29)->SetTerrainType(TerrainType::Forest);
	m_pGridGraph->GetNode(30)->SetTerrainType(TerrainType::Forest);
	m_pGridGraph->GetNode(199)->SetTerrainType(TerrainType::Forest);
	m_pGridGraph->GetNode(179)->SetTerrainType(TerrainType::Forest);
	m_pGridGraph->GetNode(159)->SetTerrainType(TerrainType::Forest);
	m_pGridGraph->GetNode(198)->SetTerrainType(TerrainType::Forest);
	m_pGridGraph->GetNode(178)->SetTerrainType(TerrainType::Forest);
	m_pGridGraph->GetNode(158)->SetTerrainType(TerrainType::Forest);
	m_pGridGraph->GetNode(197)->SetTerrainType(TerrainType::Forest);
	m_pGridGraph->GetNode(177)->SetTerrainType(TerrainType::Hill);
	m_pGridGraph->GetNode(157)->SetTerrainType(TerrainType::Forest);
	m_pGridGraph->GetNode(196)->SetTerrainType(TerrainType::Forest);
	m_pGridGraph->GetNode(176)->SetTerrainType(TerrainType::Forest);
	m_pGridGraph->GetNode(156)->SetTerrainType(TerrainType::Forest);
	m_pGridGraph->GetNode(195)->SetTerrainType(TerrainType::Forest);
	m_pGridGraph->GetNode(175)->SetTerrainType(TerrainType::Forest);
	m_pGridGraph->GetNode(155)->SetTerrainType(TerrainType::Forest);
	m_pGridGraph->GetNode(76)->SetTerrainType(TerrainType::Hill);
	m_pGridGraph->GetNode(141)->SetTerrainType(TerrainType::Hill);

	for (size_t i = 0; i < m_pGridGraph->GetNrOfNodes(); ++i)
	{
		m_pGridGraph->UnIsolateNode(i);
	}
}
