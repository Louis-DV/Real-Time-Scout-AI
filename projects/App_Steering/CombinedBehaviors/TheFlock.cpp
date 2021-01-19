#include "stdafx.h"
#include "TheFlock.h"

#include "../SteeringAgent.h"
#include "../SteeringBehaviors.h"
#include "CombinedSteeringBehaviors.h"
#include "SpacePartitioning.h"

using namespace Elite;

//Constructor & Destructor
Flock::Flock(
	int flockSize /*= 50*/, 
	float worldSize /*= 100.f*/, 
	SteeringAgent* pAgentToEvade /*= nullptr*/, 
	bool trimWorld /*= false*/)

	: m_WorldSize{ worldSize }
	, m_FlockSize{ flockSize }
	, m_TrimWorld { trimWorld }
	, m_pAgentToEvade{pAgentToEvade}
	, m_NeighborhoodRadius{ 15 }
	, m_NrOfNeighbors{0}
	, m_UseSpacePartitioning{true}
	, m_DrawDebug{false}
{
	m_Neighbors.resize(m_FlockSize);

	m_pCohesion = new Cohesion{this};
	m_pSeparation = new Separation{this};
	m_pAlignment = new Alignment{this};
	m_pSeek = new Seek{};
	m_pWander = new Wander{};
	m_pEvade = new Evade{};

	m_pBlendedSteering = new BlendedSteering({ {m_pCohesion,0.2f}, {m_pSeparation,0.3f}, {m_pAlignment, 0.2f}, {m_pSeek, 0.1f}, {m_pWander, 0.2f} });

	m_pPrioritySteering = new PrioritySteering({ m_pEvade, m_pBlendedSteering });

	m_pCellSpace = new CellSpace(m_WorldSize,m_WorldSize, 12,12,m_FlockSize);

	for (size_t i = 0; i < m_FlockSize; i++)
	{
		m_Agents.push_back(new SteeringAgent{});
		m_Agents[i]->SetPosition({ randomFloat(m_WorldSize), randomFloat(m_WorldSize) });
		m_Agents[i]->SetSteeringBehavior(m_pPrioritySteering);
		m_Agents[i]->SetMaxLinearSpeed(100.f);
		m_Agents[i]->SetAutoOrient(true);
		m_Agents[i]->SetMass(1);
		m_pCellSpace->AddAgent(m_Agents[i]);
		m_OldPosMap[m_Agents[i]] = m_Agents[i]->GetPosition();
	}
}

Flock::~Flock()
{
	SAFE_DELETE(m_pCohesion);
	SAFE_DELETE(m_pSeparation);
	SAFE_DELETE(m_pAlignment);
	SAFE_DELETE(m_pSeek);
	SAFE_DELETE(m_pEvade);
	SAFE_DELETE(m_pWander);
	SAFE_DELETE(m_pBlendedSteering);
	SAFE_DELETE(m_pPrioritySteering);
	SAFE_DELETE(m_pCellSpace);
	for (SteeringAgent* agent : m_Agents)
	{
		SAFE_DELETE(agent);
	}
}

void Flock::Update(float deltaT)
{
	// loop over all the boids
	// register its neighbors
	// update it
	// trim it to the world
	TargetData evadeTarget;
	evadeTarget.LinearVelocity = m_pAgentToEvade->GetLinearVelocity();
	evadeTarget.Position = m_pAgentToEvade->GetPosition();
	m_pEvade->SetTarget(evadeTarget);

	for (SteeringAgent* agent : m_Agents)
	{
		if (m_TrimWorld)
		{
			agent->TrimToWorld({ 0,0 }, { m_WorldSize,m_WorldSize });
		}
		
		
		if (m_UseSpacePartitioning)
		{

			m_pCellSpace->RegisterNeighbors(agent->GetPosition(), m_NeighborhoodRadius);
			m_Neighbors = m_pCellSpace->GetNeighbors();
			m_NrOfNeighbors = m_pCellSpace->GetNrOfNeighbors();
			agent->Update(deltaT);

			m_pCellSpace->UpdateAgentCell(agent, m_OldPosMap[agent]);
			m_OldPosMap[agent] = agent->GetPosition();
		}
		else
		{
			RegisterNeighbors(agent);
			agent->Update(deltaT);
		}
		
		
		
		
		
		if (m_DrawDebug && agent == m_Agents[0])
		{
			for (SteeringAgent* agent : m_Agents)
			{
				agent->SetBodyColor({1.f, 1.f, 0.f});
			}
			DEBUGRENDERER2D->DrawCircle(agent->GetPosition(), m_NeighborhoodRadius, { 1,1,1 }, 0.5f);
			for (size_t i = 0; i < m_NrOfNeighbors; i++)
			{
				m_Neighbors[i]->SetBodyColor({ 0.f,1.f,0.f });
			}
		}
	}

}

void Flock::Render(float deltaT)
{
	if (m_DrawDebug)
	{
		for (SteeringAgent* agent : m_Agents)
		{
			agent->Render(deltaT);
		}
		if (m_UseSpacePartitioning)
		{
			m_pCellSpace->RenderCells();
			m_pCellSpace->DebugPartitioning(m_Agents[0]->GetPosition(), m_NeighborhoodRadius);
		}
	}
}

void Flock::UpdateAndRenderUI()
{
	//Setup
	int menuWidth = 235;
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
	ImGui::Text("LMB: place target");
	ImGui::Text("RMB: move cam.");
	ImGui::Text("Scrollwheel: zoom cam.");
	ImGui::Unindent();

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();
	ImGui::Spacing();

	ImGui::Text("STATS");
	ImGui::Indent();
	ImGui::Text("%.3f ms/frame", 1000.0f / ImGui::GetIO().Framerate);
	ImGui::Text("%.1f FPS", ImGui::GetIO().Framerate);
	ImGui::Unindent();

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	ImGui::Text("Flocking");
	ImGui::Spacing();

	// Implement checkboxes and sliders here
	ImGui::Text("Behavior Weights");
	ImGui::Spacing();

	ImGui::SliderFloat("Cohesion", &m_pBlendedSteering->m_WeightedBehaviors[0].weight, 0.f, 1.f, "%.2");
	ImGui::SliderFloat("Separation", &m_pBlendedSteering->m_WeightedBehaviors[1].weight, 0.f, 1.f, "%.2");
	ImGui::SliderFloat("Alignment", &m_pBlendedSteering->m_WeightedBehaviors[2].weight, 0.f, 1.f, "%.2");
	ImGui::SliderFloat("Seek", &m_pBlendedSteering->m_WeightedBehaviors[3].weight, 0.f, 1.f, "%.2");
	ImGui::SliderFloat("Wander", &m_pBlendedSteering->m_WeightedBehaviors[4].weight, 0.f, 1.f, "%.2");

	ImGui::Checkbox("Partitioning", &m_UseSpacePartitioning);
	ImGui::Checkbox("Debug", &m_DrawDebug);

	//End
	ImGui::PopAllowKeyboardFocus();
	ImGui::End();
}

void Flock::RegisterNeighbors(SteeringAgent* pAgent)
{
	// register the agents neighboring the currently evaluated agent
	// store how many they are, so you know which part of the vector to loop over
	m_NrOfNeighbors = 0;
	auto pos = pAgent->GetPosition();
	for (SteeringAgent* agent : m_Agents)
	{
		if (agent != pAgent && DistanceSquared(agent->GetPosition(), pos) < Square( m_NeighborhoodRadius))
		{
			m_Neighbors[m_NrOfNeighbors] = agent;
			++m_NrOfNeighbors;
		}
	}
}

Elite::Vector2 Flock::GetAverageNeighborPos() const
{
	float x{};
	float y{};

	for (size_t i = 0; i < m_NrOfNeighbors; i++)
	{
		x += m_Neighbors[i]->GetPosition().x;
		y += m_Neighbors[i]->GetPosition().y;
		
	}


	return Elite::Vector2{x/m_NrOfNeighbors,y/m_NrOfNeighbors};

}

Elite::Vector2 Flock::GetAverageNeighborVelocity() const
{
	float x{};
	float y{};

	for (size_t i = 0; i < m_NrOfNeighbors; i++)
	{
		x += m_Neighbors[i]->GetLinearVelocity().x;
		y += m_Neighbors[i]->GetLinearVelocity().y;

	}


	return Elite::Vector2{ x / m_NrOfNeighbors,y / m_NrOfNeighbors };

}


float* Flock::GetWeight(ISteeringBehavior* pBehavior) 
{
	if (m_pBlendedSteering)
	{
		auto& weightedBehaviors = m_pBlendedSteering->m_WeightedBehaviors;
		auto it = find_if(weightedBehaviors.begin(),
			weightedBehaviors.end(),
			[pBehavior](BlendedSteering::WeightedBehavior el)
			{
				return el.pBehavior == pBehavior;
			}
		);

		if(it!= weightedBehaviors.end())
			return &it->weight;
	}

	return nullptr;
}
