#include "stdafx.h"
#include "FlockingSteeringBehaviors.h"
#include "TheFlock.h"
#include "../SteeringAgent.h"
#include "../SteeringHelpers.h"

//*********************
//SEPARATION (FLOCKING)
SteeringOutput Separation::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{

	if (m_pFlock->GetNrOfNeighbors() > 0)
	{
		SteeringOutput steering = {};
		auto neighbors = m_pFlock->GetNeighbors();
		float maxDist = m_pFlock->GetNeighborhoodRadius();

		Elite::Vector2 direction{};

		for (size_t i = 0; i < m_pFlock->GetNrOfNeighbors(); i++)
		{
			direction = pAgent->GetPosition() - neighbors[i]->GetPosition();
			Normalize(direction);
			steering.LinearVelocity += direction * (maxDist - Distance(pAgent->GetPosition(), neighbors[i]->GetPosition())) / maxDist;
		}

		if (steering.LinearVelocity.MagnitudeSquared() > 1)
		{
			steering.LinearVelocity /= steering.LinearVelocity.Magnitude();
		}
		steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();

		if (pAgent->CanRenderBehavior())
		{
			DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, 5, { 0,1,0,0.5f }, 0.40f);
		}

		return steering;
	}
	else
	{
		SteeringOutput steering;
		steering.IsValid = false;
		return steering;
	}
}


//*******************
//COHESION (FLOCKING)
SteeringOutput Cohesion::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};

	steering.LinearVelocity = m_pFlock->GetAverageNeighborPos() - pAgent->GetPosition();
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();

	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, 5, { 0,1,0,0.5f }, 0.40f);
	}

	return steering;
}



//*************************
//VELOCITY MATCH (FLOCKING)
SteeringOutput Alignment::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{

	if (m_pFlock->GetAverageNeighborVelocity() != Elite::ZeroVector2)
	{

		SteeringOutput steering = {};

		steering.LinearVelocity = m_pFlock->GetAverageNeighborVelocity();

		if (pAgent->CanRenderBehavior())
		{
			DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, 5, { 0,1,0,0.5f }, 0.40f);
		}

		return steering;
	
	}
	else
	{
		SteeringOutput steering;
		steering.IsValid = false;
		return steering;
	}
}
