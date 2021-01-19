//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

//Includes
#include "SteeringBehaviors.h"
#include "SteeringAgent.h"

//SEEK
//****
SteeringOutput Seek::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};

	steering.LinearVelocity = m_Target.Position - pAgent->GetPosition();
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();

	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, 5, { 0,1,0,0.5f }, 0.40f);
	}

	return steering;
}

//WANDER (base> SEEK)
//******
SteeringOutput Wander::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{

	SteeringOutput steering = {};
	Elite::Vector2 circleCenter{};
	Elite::Vector2 target{};
	m_WanderAngle += Elite::randomFloat(-m_AngleChange,m_AngleChange);
	if (m_WanderAngle > ToRadians(180))
	{
		m_WanderAngle -= ToRadians(360);
	}
	else if(m_WanderAngle < ToRadians(-180))
	{
		m_WanderAngle += ToRadians(360);
	}
	circleCenter = pAgent->GetPosition() + pAgent->GetDirection() * m_Offset;
	target.x = circleCenter.x + m_Radius * cos(m_WanderAngle);
	target.y = circleCenter.y + m_Radius * sin(m_WanderAngle);
	steering.LinearVelocity = target - pAgent->GetPosition();
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();

	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, 5, { 0,1,0,0.5f }, 0.40f);
		DEBUGRENDERER2D->DrawCircle(circleCenter, m_Radius, { 0,0,1,0.5f }, 0.20f);
		DEBUGRENDERER2D->DrawPoint(target, m_Radius, { 1,0,0,0.5f }, 0.20f);
	}

	return steering;

	return SteeringOutput{};
}

SteeringOutput Flee::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};

	steering.LinearVelocity = pAgent->GetPosition() - (m_Target).Position;
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();

	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, 5, { 0,1,0,0.5f }, 0.40f);
	}

	return steering;
}

SteeringOutput Arrive::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{

	SteeringOutput steering = {};

	steering.LinearVelocity = (m_Target).Position - pAgent->GetPosition();
	steering.LinearVelocity.Normalize();

	if (pAgent->GetPosition().Distance((m_Target).Position) < m_TargetRadius)
	{
		steering.LinearVelocity = Elite::ZeroVector2;
		return steering;
	}

	if (pAgent->GetPosition().Distance((m_Target).Position) <= m_SlowRadius)
	{
		steering.LinearVelocity *= pAgent->GetMaxLinearSpeed() * (pAgent->GetPosition().Distance((m_Target).Position) / m_SlowRadius);
	}
	else
	{

		steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();

	}

	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, 5, { 0,1,0,0.5f }, 0.40f);
	}

	return steering;
}

void Arrive::SetSlowRadius(float slowRadius)
{
	m_SlowRadius = slowRadius;
}

void Arrive::SetTargetRadius(float targetRadius)
{
	m_TargetRadius = targetRadius;
}

SteeringOutput Face::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};

	pAgent->SetAutoOrient(false);
	float angle = acos(Dot(m_Target.Position, pAgent->GetPosition()) / (m_Target.Position.Magnitude() * pAgent->GetPosition().Magnitude()));
	angle -= pAgent->GetRotation()*(180/3.1415);
	std::cout << pAgent->GetRotation();

	steering.AngularVelocity = angle;

	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, 5, { 0,1,0,0.5f }, 0.40f);
	}

	return steering;
}

SteeringOutput Pursuit::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};

	steering.LinearVelocity = m_Target.Position + m_Target.LinearVelocity - pAgent->GetPosition();
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();

	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, 5, { 0,1,0,0.5f }, 0.40f);
		DEBUGRENDERER2D->DrawPoint(m_Target.Position + m_Target.LinearVelocity, 5, { 1,0,0,0.5f }, 0.20f);
	}

	return steering;
}

SteeringOutput Evade::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	auto distanceToTarget = Distance(pAgent->GetPosition(), m_Target.Position);

	if (distanceToTarget> m_FleeRadius)
	{
		SteeringOutput steering;
		steering.IsValid = false;
		return steering;
	}

	SteeringOutput steering = {};
	steering.LinearVelocity = pAgent->GetPosition() - (m_Target.Position + (m_Target.LinearVelocity/m_Target.LinearVelocity.Magnitude()*5));
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();

	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, 5, { 0,1,0,0.5f }, 0.40f);
		DEBUGRENDERER2D->DrawPoint(m_Target.Position + (m_Target.LinearVelocity / m_Target.LinearVelocity.Magnitude()*5), 5, { 1,0,0,0.5f }, 0.20f);
	}

	return steering;
}
