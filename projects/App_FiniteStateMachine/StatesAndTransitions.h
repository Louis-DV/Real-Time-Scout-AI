/*=============================================================================*/
// Copyright 2020-2021 Elite Engine
/*=============================================================================*/
// StatesAndTransitions.h: Implementation of the state/transition classes
/*=============================================================================*/
#ifndef ELITE_APPLICATION_FSM_STATES_TRANSITIONS
#define ELITE_APPLICATION_FSM_STATES_TRANSITIONS

#include "../Shared/Agario/AgarioAgent.h"
#include "../Shared/Agario/AgarioFood.h"
#include "../App_Steering/SteeringBehaviors.h"

class WanderState : public Elite::FSMState
{
public:
	WanderState() : FSMState() {};
	virtual void OnEnter(Blackboard* pB)
	{
		AgarioAgent* pAgent = nullptr;
		bool dataAvailable = pB->GetData("Agent", pAgent);
	
		if (!dataAvailable)
			return;
		if (!pAgent)
			return;


		pAgent->SetToWander();
	}
};


class SeekToFoodState : public Elite::FSMState
{
public:
	SeekToFoodState() : FSMState() {};
	/*virtual void OnEnter(Blackboard* pBlackboard)
	{
		AgarioAgent* pAgent = nullptr;
		bool dataAvailable = pBlackboard->GetData("Agent", pAgent);
		if (!dataAvailable)
			return;
		std::vector<AgarioFood*>* pFoodVec = nullptr;
		dataAvailable = pBlackboard->GetData("FoodVec", pFoodVec);
		if (!dataAvailable)
			return;
	
		if (!pAgent)
			return;
		if (!pFoodVec)
			return;

		Elite::Vector2 closestPos{FLT_MAX,FLT_MAX};

		for (AgarioFood* food : *pFoodVec)
		{
			if (DistanceSquared(pAgent->GetPosition(),closestPos) > DistanceSquared(pAgent->GetPosition(),food->GetPosition()))
			{
				closestPos = food->GetPosition();
			}
		}

		pAgent->SetToSeek(closestPos);
	}*/

	virtual void Update(Blackboard* pBlackboard, float deltaTime)
	{
		AgarioAgent* pAgent = nullptr;
		bool dataAvailable = pBlackboard->GetData("Agent", pAgent);
		if (!dataAvailable)
			return;
		std::vector<AgarioFood*>* pFoodVec = nullptr;
		dataAvailable = pBlackboard->GetData("FoodVec", pFoodVec);
		if (!dataAvailable)
			return;

		if (!pAgent)
			return;
		if (!pFoodVec)
			return;

		Elite::Vector2 closestPos{ FLT_MAX,FLT_MAX };

		for (AgarioFood* food : *pFoodVec)
		{
			if (DistanceSquared(pAgent->GetPosition(), closestPos) > DistanceSquared(pAgent->GetPosition(), food->GetPosition()))
			{
				closestPos = food->GetPosition();
			}
		}

		pAgent->SetToSeek(closestPos);
	}
};

class FleeFromAgentState : public Elite::FSMState
{
public:
	FleeFromAgentState() : FSMState() {};
	/*virtual void OnEnter(Blackboard* pBlackboard)
	{
		AgarioAgent* pAgent = nullptr;
		bool dataAvailable = pBlackboard->GetData("Agent", pAgent);
		if (!dataAvailable)
			return;
		std::vector<AgarioAgent*>* pAgentVec = nullptr;
		dataAvailable = pBlackboard->GetData("AgentVec", pAgentVec);
		if (!dataAvailable)
			return;
	
		if (!pAgent)
			return;
		if (!pAgentVec)
			return;

		Elite::Vector2 closestPos{FLT_MAX,FLT_MAX};

		for (AgarioAgent* agent : *pAgentVec)
		{
			if (DistanceSquared(pAgent->GetPosition(),closestPos) > DistanceSquared(pAgent->GetPosition(),food->GetPosition()))
			{
				closestPos = food->GetPosition();
			}
		}

		pAgent->SetToFlee(closestPos);
	}*/

	virtual void Update(Blackboard* pBlackboard, float deltaTime)
	{
		AgarioAgent* pAgent = nullptr;
		bool dataAvailable = pBlackboard->GetData("Agent", pAgent);
		if (!dataAvailable)
			return;
		std::vector<AgarioAgent*>* pAgentVec = nullptr;
		dataAvailable = pBlackboard->GetData("AgentVec", pAgentVec);
		if (!dataAvailable)
			return;

		if (!pAgent)
			return;
		if (!pAgentVec)
			return;

		Elite::Vector2 closestPos{ FLT_MAX,FLT_MAX };

		for (AgarioAgent* agent : *pAgentVec)
		{
			if (agent->GetRadius() > pAgent->GetRadius() && DistanceSquared(pAgent->GetPosition(), closestPos) > DistanceSquared(pAgent->GetPosition(), agent->GetPosition()))
			{
				closestPos = agent->GetPosition();
			}
		}

		pAgent->SetToFlee(closestPos);
	}
};


class CloseToFood : public Elite::FSMTransition
{
public:
	CloseToFood() : FSMTransition() {}
	virtual bool ToTransition(Blackboard* pBlackboard) const override
	{
		AgarioAgent* pAgent = nullptr;
		bool dataAvailable = pBlackboard->GetData("Agent", pAgent);

		std::vector<AgarioFood*>* pFoodVec = nullptr;
		pBlackboard->GetData("FoodVec", pFoodVec);
		
		for (AgarioFood* food : *pFoodVec)
		{
			if (DistanceSquared(pAgent->GetPosition(), food->GetPosition())<(1000+(pAgent->GetRadius()* pAgent->GetRadius())))
			{
				return true;
			}

		}
		return false;
	}


};

class CloseToBiggerAgent : public Elite::FSMTransition
{
public:
	CloseToBiggerAgent() : FSMTransition() {}
	virtual bool ToTransition(Blackboard* pBlackboard) const override
	{
		AgarioAgent* pAgent = nullptr;
		pBlackboard->GetData("Agent", pAgent);

		std::vector<AgarioAgent*>* pAgentVec = nullptr;
		pBlackboard->GetData("AgentVec", pAgentVec);
		
		for (AgarioAgent* agent : *pAgentVec)
		{
			if (agent->GetRadius() > pAgent->GetRadius() && DistanceSquared(pAgent->GetPosition(), agent->GetPosition())<(1000+(pAgent->GetRadius()* pAgent->GetRadius())))
			{
				return true;
			}

		}
		return false;
	}


};

class FleeToWander : public Elite::FSMTransition
{
public:
	FleeToWander() : FSMTransition() {}
	virtual bool ToTransition(Blackboard* pBlackboard) const override
	{
		AgarioAgent* pAgent = nullptr;
		pBlackboard->GetData("Agent", pAgent);

		std::vector<AgarioAgent*>* pAgentVec = nullptr;
		pBlackboard->GetData("AgentVec", pAgentVec);
		
		for (AgarioAgent* agent : *pAgentVec)
		{
			if (agent->GetRadius() > pAgent->GetRadius() && DistanceSquared(pAgent->GetPosition(), agent->GetPosition())<(1000+(pAgent->GetRadius()* pAgent->GetRadius())))
			{
				return false;
			}

		}

		return true;
	}


};

class SeekFoodToWander : public Elite::FSMTransition
{
public:
	SeekFoodToWander() : FSMTransition() {}
	virtual bool ToTransition(Blackboard* pBlackboard) const override
	{
		AgarioAgent* pAgent = nullptr;
		pBlackboard->GetData("Agent", pAgent);

		std::vector<AgarioFood*>* pFoodVec = nullptr;
		pBlackboard->GetData("FoodVec", pFoodVec);

		for (AgarioFood* food : *pFoodVec)
		{
			if (DistanceSquared(pAgent->GetPosition(), food->GetPosition()) < (1000 + (pAgent->GetRadius() * pAgent->GetRadius())))
			{
				return false;
			}

		}

		return true;
	}


};


#endif