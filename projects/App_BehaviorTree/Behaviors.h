/*=============================================================================*/
// Copyright 2020-2021 Elite Engine
/*=============================================================================*/
// Behaviors.h: Implementation of certain reusable behaviors for the BT version of the Agario Game
/*=============================================================================*/
#ifndef ELITE_APPLICATION_BEHAVIOR_TREE_BEHAVIORS
#define ELITE_APPLICATION_BEHAVIOR_TREE_BEHAVIORS
//-----------------------------------------------------------------
// Includes & Forward Declarations
//-----------------------------------------------------------------
#include "framework/EliteMath/EMath.h"
#include "framework/EliteAI/EliteDecisionMaking/EliteBehaviorTree/EBehaviorTree.h"
#include "../Shared/Agario/AgarioAgent.h"
#include "../Shared/Agario/AgarioFood.h"
#include "../App_Steering/SteeringBehaviors.h"

//-----------------------------------------------------------------
// Behaviors
//-----------------------------------------------------------------
bool IsCloseToFood(Elite::Blackboard* pBlackboard)
{
	AgarioAgent* pAgent = nullptr;
	std::vector<AgarioFood*>* foodVec = nullptr;

	auto dataAvailable = pBlackboard->GetData("Agent", pAgent) &&
		pBlackboard->GetData("FoodVec", foodVec);

	if (!pAgent || !foodVec)
		return false;

	//TODO: Check for food closeby and set target accordingly
	const float closeToFoodRange{ 20.f };
	auto foodIt = std::find_if((*foodVec).begin(), (*foodVec).end(), [&pAgent,&closeToFoodRange](AgarioFood* f) 
		{
			return DistanceSquared(pAgent->GetPosition(), f->GetPosition()) < (closeToFoodRange * closeToFoodRange);
		});

	if (foodIt != (*foodVec).end())
	{
		pBlackboard->ChangeData("Target", (*foodIt)->GetPosition());
		return true;
	}

	return false;
}

bool IsCloseToBiggerEnemy(Elite::Blackboard* pBlackboard)
{
	AgarioAgent* pAgent = nullptr;
	std::vector<AgarioAgent*>* agentVec = nullptr;

	auto dataAvailable = pBlackboard->GetData("Agent", pAgent) &&
		pBlackboard->GetData("AgentsVec", agentVec);

	if (!pAgent || !agentVec)
		return false;

	//TODO: Check for food closeby and set target accordingly
	const float closeToAgentRange{ 20.f };
	auto agentIt = std::find_if((*agentVec).begin(), (*agentVec).end(), [&pAgent,&closeToAgentRange](AgarioAgent* a) 
		{
			return (a->GetRadius() > pAgent->GetRadius() && DistanceSquared(pAgent->GetPosition(), a->GetPosition()) < (closeToAgentRange * closeToAgentRange));
		});

	if (agentIt != (*agentVec).end())
	{
		pBlackboard->ChangeData("Target", (*agentIt)->GetPosition());
		return true;
	}

	return false;
}



BehaviorState ChangeToWander(Elite::Blackboard* pBlackboard)
{
	AgarioAgent* pAgent = nullptr;
	auto dataAvailable = pBlackboard->GetData("Agent", pAgent);

	if (!pAgent)
		return Failure;

	pAgent->SetToWander();

	return Success;
}

BehaviorState ChangeToSeek(Elite::Blackboard* pBlackboard)
{
	AgarioAgent* pAgent = nullptr;
	Elite::Vector2 seekTarget{};
	auto dataAvailable = pBlackboard->GetData("Agent", pAgent) &&
		pBlackboard->GetData("Target", seekTarget);


	if (!pAgent || !dataAvailable)
		return Failure;
	
	//TODO: Implement Change to seek (Target)

	pAgent->SetToSeek(seekTarget);

	return Success;
}

BehaviorState ChangeToFlee(Elite::Blackboard* pBlackboard)
{
	AgarioAgent* pAgent = nullptr;
	Elite::Vector2 fleeTarget{};
	auto dataAvailable = pBlackboard->GetData("Agent", pAgent) &&
		pBlackboard->GetData("Target", fleeTarget);


	if (!pAgent || !dataAvailable)
		return Failure;
	
	//TODO: Implement Change to seek (Target)

	pAgent->SetToFlee(fleeTarget);

	return Success;
}

#endif