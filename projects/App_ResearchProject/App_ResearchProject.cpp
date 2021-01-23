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



	if (m_pGridGraph->GetNode(m_pGridGraph->GetNodeFromWorldPos(m_pAgent->GetPosition()))->GetTerrainType() == TerrainType::Hill)
	{
		m_SightRadius = 35.f;
		SetHillScore();
	}
	else
	{
		m_SightRadius = 25.f;
	}


	for (GridTerrainNode* node : m_pMappedGridGraph->GetAllNodes())
	{
		if (DistanceSquared(m_pMappedGridGraph->GetNodeWorldPos(node->GetIndex()), m_pAgent->GetPosition()) <= m_SightRadius * m_SightRadius)
		{


			node->SetTerrainType(m_pGridGraph->GetNode(node->GetIndex())->GetTerrainType());
			m_pMappedGridGraph->UnIsolateNode(node->GetIndex());
		}
	}

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

			if ((node->GetTerrainType() == TerrainType::Hill))
			{
				m_ImportantNodeList.push_back(node->GetIndex());
				endPathIdx = node->GetIndex();
			}

			auto it = std::find(m_ImportantNodeList.begin(), m_ImportantNodeList.end(), node->GetIndex());
			if (it != m_ImportantNodeList.end())
			{
				if (!important)
				{
					m_ImportantNodeList.erase(it);
					m_SeenNodesMap[node->GetIndex()] = 0.f;
				}
			}
			else
			{
				if (important)
				{
					m_ImportantNodeList.push_back(node->GetIndex());
				}
				else
				{
					m_SeenNodesMap[node->GetIndex()] = 0.f;
				}
			}


			


		}
	}

	if (!m_ImportantNodeList.empty())
	{
		int newEndPathIdx = *std::min_element(m_ImportantNodeList.begin(), m_ImportantNodeList.end(), [this](int idx1, int idx2) {return DistanceSquared(m_pAgent->GetPosition(), m_pMappedGridGraph->GetNodeWorldPos(idx1)) < DistanceSquared(m_pAgent->GetPosition(), m_pMappedGridGraph->GetNodeWorldPos(idx2)); });
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
	//Render grid
	/*m_GraphRenderer.RenderGraph(
		m_pGridGraph, 
		m_bDrawGrid, 
		m_bDrawNodeNumbers, 
		m_bDrawConnections, 
		m_bDrawConnectionsCosts
	);*/
	m_GraphRenderer.RenderGraph(
		m_pMappedGridGraph, 
		m_bDrawGrid, 
		m_bDrawNodeNumbers, 
		m_bDrawConnections, 
		m_bDrawConnectionsCosts
	);
	
	//Render start node on top if applicable
	if (startPathIdx != invalid_node_index)
	{
		m_GraphRenderer.RenderHighlightedGrid(m_pGridGraph, { m_pGridGraph->GetNode(startPathIdx) }, START_NODE_COLOR);
	}

	//Render end node on top if applicable
	if (endPathIdx != invalid_node_index)
	{
		m_GraphRenderer.RenderHighlightedGrid(m_pGridGraph, { m_pGridGraph->GetNode(endPathIdx) }, END_NODE_COLOR);
	}
	
	//render path below if applicable
	if (m_vPath.size() > 0)
	{
		m_GraphRenderer.RenderHighlightedGrid(m_pGridGraph, m_vPath);
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



	//m_pGridGraph->IsolateNode(6);
	/*m_pGridGraph->GetNode(7)->SetTerrainType(TerrainType::Mud);
	m_pGridGraph->GetNode(8)->SetTerrainType(TerrainType::Wall);
	m_pGridGraph->GetNode(9)->SetTerrainType(TerrainType::Water);*/
	for (size_t i = 0; i < m_pGridGraph->GetNrOfNodes(); ++i)
	{
		m_pGridGraph->UnIsolateNode(i);
	}
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

}
