#pragma once
#include "../SteeringHelpers.h"
#include "FlockingSteeringBehaviors.h"
#include <map>

class ISteeringBehavior;
class SteeringAgent;
class BlendedSteering;
class PrioritySteering;
class CellSpace;

class Flock
{
public:
	Flock(
		int flockSize = 50, 
		float worldSize = 100.f, 
		SteeringAgent* pAgentToEvade = nullptr, 
		bool trimWorld = false);

	~Flock();

	void Update(float deltaT);
	void UpdateAndRenderUI();
	void Render(float deltaT);

	void RegisterNeighbors(SteeringAgent* pAgent);
	int GetNrOfNeighbors() const { return m_NrOfNeighbors; }
	const vector<SteeringAgent*>& GetNeighbors() const { return m_Neighbors; }
	float GetNeighborhoodRadius() { return m_NeighborhoodRadius; }

	Elite::Vector2 GetAverageNeighborPos() const;
	Elite::Vector2 GetAverageNeighborVelocity() const;

	void SetSeekTarget(const TargetData& target) { m_pSeek->SetTarget(target); }

private:
	// flock agents
	int m_FlockSize = 0;
	vector<SteeringAgent*> m_Agents;

	// neighborhood agents
	vector<SteeringAgent*> m_Neighbors;
	float m_NeighborhoodRadius = 10.f;
	int m_NrOfNeighbors = 0;

	// evade target
	SteeringAgent* m_pAgentToEvade = nullptr;

	// world info
	bool m_TrimWorld = false;
	float m_WorldSize = 0.f;

	CellSpace* m_pCellSpace = nullptr;
	bool m_UseSpacePartitioning;
	bool m_DrawDebug;
	std::map<SteeringAgent*,Elite::Vector2> m_OldPosMap;

	// steering Behaviors
	BlendedSteering* m_pBlendedSteering = nullptr;
	PrioritySteering* m_pPrioritySteering = nullptr;
	Cohesion* m_pCohesion = nullptr;
	Separation* m_pSeparation = nullptr;
	Alignment* m_pAlignment = nullptr;
	Seek* m_pSeek = nullptr;
	Wander* m_pWander = nullptr;
	Evade* m_pEvade = nullptr;


	// private functions
	float* GetWeight(ISteeringBehavior* pBehaviour);

private:
	Flock(const Flock& other);
	Flock& operator=(const Flock& other);
};