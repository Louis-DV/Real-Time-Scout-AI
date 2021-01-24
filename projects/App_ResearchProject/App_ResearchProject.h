#ifndef ASTAR_APPLICATION_H
#define ASTAR_APPLICATION_H
//-----------------------------------------------------------------
// Includes & Forward Declarations
//-----------------------------------------------------------------
#include "framework/EliteInterfaces/EIApp.h"
#include "framework\EliteAI\EliteGraphs\EGridGraph.h"
#include "framework\EliteAI\EliteGraphs\EliteGraphUtilities\EGraphEditor.h"
#include "framework\EliteAI\EliteGraphs\EliteGraphUtilities\EGraphRenderer.h"
#include "framework/EliteInterfaces/EIApp.h"
#include "../App_Steering/SteeringHelpers.h"
#include "../App_Steering/SteeringAgent.h"
#include "../App_Steering/SteeringBehaviors.h"
#include <map>

//-----------------------------------------------------------------
// Application
//-----------------------------------------------------------------
class App_ResearchProject final : public IApp
{
public:
	//Constructor & Destructor
	App_ResearchProject() = default;
	virtual ~App_ResearchProject();

	//App Functions
	void Start() override;
	void Update(float deltaTime) override;
	void Render(float deltaTime) const override;

private:
	//Datamembers
	const bool ALLOW_DIAGONAL_MOVEMENT = true;
	Elite::Vector2 m_StartPosition = Elite::ZeroVector2;
	Elite::Vector2 m_TargetPosition = Elite::ZeroVector2;

	SteeringAgent* m_pAgent = nullptr;
	Seek* m_pSeekBehavior = nullptr;
	float m_AgentSpeed = 10.f;
	float m_SightRadius = 25.f;
	std::map<int, int> m_SeenNodesMap;
	std::list<int> m_ImportantNodeList;

	//Grid datamembers
	static const int COLUMNS = 20;
	static const int ROWS = 10;
	unsigned int m_SizeCell = 10;
	Elite::GridGraph<Elite::GridTerrainNode, Elite::GraphConnection>* m_pGridGraph;
	Elite::GridGraph<Elite::GridTerrainNode, Elite::GraphConnection>* m_pMappedGridGraph;


	//Pathfinding datamembers
	int startPathIdx = invalid_node_index;
	int endPathIdx = invalid_node_index;
	std::vector<Elite::GridTerrainNode*> m_vPath;
	bool m_UpdatePath = true;

	//Editor and Visualisation
	Elite::EGraphEditor m_GraphEditor{};
	Elite::EGraphRenderer m_GraphRenderer{};

	//Debug rendering information
	bool m_bDrawGrid = true;
	bool m_bDrawNodeNumbers = false;
	bool m_bDrawConnections = false;
	bool m_bDrawConnectionsCosts = false;
	bool m_StartSelected = true;
	int m_SelectedHeuristic = 4;
	Elite::Heuristic m_pHeuristicFunction = Elite::HeuristicFunctions::Chebyshev;

	//Functions
	void MakeGridGraph();
	void UpdateImGui();
	void SetHillScore();
	void SetForestScore();
	bool SetNodeScore(int idx);
	void CreateMap();

	//C++ make the class non-copyable
	App_ResearchProject(const App_ResearchProject&) = delete;
	App_ResearchProject& operator=(const App_ResearchProject&) = delete;
};
#endif