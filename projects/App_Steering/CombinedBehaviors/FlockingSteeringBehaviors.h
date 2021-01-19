#pragma once
#include "../SteeringBehaviors.h"

class Flock;

//SEPARATION - FLOCKING
//*********************
class Separation : public Flee
{
public:
	Separation(Flock* pFlock) : m_pFlock{ pFlock } {};
	virtual ~Separation() = default;

	//Seek Behaviour
	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;

private:
	Flock* m_pFlock;
};

//COHESION - FLOCKING
//*******************
class Cohesion : public Seek
{
public:
	Cohesion(Flock* pFlock): m_pFlock{pFlock} {};
	virtual ~Cohesion() = default;

	//Seek Behaviour
	SteeringOutput CalculateSteering(float deltaT, SteeringAgent * pAgent) override;

private:
	Flock* m_pFlock;
};

//VELOCITY MATCH - FLOCKING
//************************
class Alignment : public Seek
{
public:
	Alignment(Flock* pFlock) : m_pFlock{ pFlock } {};
	virtual ~Alignment() = default;

	//Seek Behaviour
	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;

private:
	Flock* m_pFlock;
};