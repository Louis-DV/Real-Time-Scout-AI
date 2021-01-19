#include "stdafx.h"
#include "SpacePartitioning.h"
#include "projects\App_Steering\SteeringAgent.h"

// --- Cell ---
// ------------
Cell::Cell(float left, float bottom, float width, float height)
{
	boundingBox.bottomLeft = { left, bottom };
	boundingBox.width = width;
	boundingBox.height = height;
}

std::vector<Elite::Vector2> Cell::GetRectPoints() const
{
	auto left = boundingBox.bottomLeft.x;
	auto bottom = boundingBox.bottomLeft.y;
	auto width = boundingBox.width;
	auto height = boundingBox.height;

	std::vector<Elite::Vector2> rectPoints =
	{
		{ left , bottom  },
		{ left , bottom + height  },
		{ left + width , bottom + height },
		{ left + width , bottom  },
	};

	return rectPoints;
}

// --- Partitioned Space ---
// -------------------------
CellSpace::CellSpace(float width, float height, int rows, int cols, int maxEntities)
	: m_SpaceWidth(width)
	, m_SpaceHeight(height)
	, m_NrOfRows(rows)
	, m_NrOfCols(cols)
	, m_Neighbors(maxEntities)
	, m_NrOfNeighbors(0)
{
	m_CellWidth = m_SpaceWidth / m_NrOfCols;
	m_CellHeight = m_SpaceHeight / m_NrOfRows;

	for (size_t i = 0; i < m_NrOfRows; i++)
	{
		for (size_t j = 0; j < m_NrOfCols; j++)
		{
			m_Cells.push_back(Cell{ j * m_CellWidth,i * m_CellHeight,m_CellWidth,m_CellHeight });
		}

	}
}

void CellSpace::AddAgent(SteeringAgent* agent)
{
	m_Cells[PositionToIndex(agent->GetPosition())].agents.push_back(agent);
}

void CellSpace::UpdateAgentCell(SteeringAgent* agent, const Elite::Vector2& oldPos)
{
	if (PositionToIndex(oldPos) != PositionToIndex(agent->GetPosition()))
	{
		AddAgent(agent);
		m_Cells[PositionToIndex(oldPos)].agents.remove(agent);
	}
}

void CellSpace::RegisterNeighbors(const Elite::Vector2& pos, float queryRadius)
{
	m_NrOfNeighbors = 0;
	Elite::Rect boundingBox{ {pos.x - queryRadius , pos.y - queryRadius} ,queryRadius * 2, queryRadius * 2 };
	
	for (size_t j = Elite::Clamp(PositionToIndex({ Elite::Clamp(boundingBox.bottomLeft.x,0.f,m_SpaceWidth), Elite::Clamp(boundingBox.bottomLeft.y,0.f,m_SpaceHeight) }) / m_NrOfCols, 0, m_NrOfRows - 1); j <= Elite::Clamp(PositionToIndex({ Elite::Clamp((boundingBox.bottomLeft + Elite::Vector2{ 0, boundingBox.height }).x,0.f,m_SpaceWidth), Elite::Clamp((boundingBox.bottomLeft + Elite::Vector2{ 0, boundingBox.height }).y,0.f,m_SpaceHeight) }) / m_NrOfCols, 0, m_NrOfRows - 1); j++)
	{
		for (size_t i = Elite::Clamp(PositionToIndex({ Elite::Clamp(boundingBox.bottomLeft.x,0.f,m_SpaceWidth), Elite::Clamp(boundingBox.bottomLeft.y,0.f,m_SpaceHeight) })%m_NrOfCols, 0, m_NrOfCols- 1); i <= Elite::Clamp(PositionToIndex({ Elite::Clamp((boundingBox.bottomLeft + Elite::Vector2{boundingBox.width, 0}).x,0.f,m_SpaceWidth), Elite::Clamp((boundingBox.bottomLeft + Elite::Vector2{boundingBox.width, 0}).y,0.f,m_SpaceHeight) }) % m_NrOfCols, 0, m_NrOfCols- 1); i++)
		{
			for (SteeringAgent* agent : m_Cells[j*m_NrOfCols+i].agents)
			{
				if (Elite::DistanceSquared(pos,agent->GetPosition()) <= Elite::Square(queryRadius) && Elite::DistanceSquared(pos, agent->GetPosition()) != 0)
				{

					m_Neighbors[m_NrOfNeighbors] = agent;
					++m_NrOfNeighbors;
				
				}
			}
		}
	}
}

void CellSpace::RenderCells() const
{
	for (Cell cell : m_Cells)
	{
		DEBUGRENDERER2D->DrawPolygon(&Elite::Polygon{ cell.GetRectPoints() }, { 1,0,0 }, 0.5f);
		DEBUGRENDERER2D->DrawString(cell.boundingBox.bottomLeft + Elite::Vector2{0,cell.boundingBox.height}, std::to_string(cell.agents.size()).c_str());
	}
}

void CellSpace::DebugPartitioning(const Elite::Vector2& pos, float queryRadius) const
{
	Elite::Rect boundingBox{ {pos.x - queryRadius , pos.y - queryRadius} ,queryRadius * 2, queryRadius * 2 };

	for (size_t j = Elite::Clamp(PositionToIndex({ Elite::Clamp(boundingBox.bottomLeft.x,0.f,m_SpaceWidth), Elite::Clamp(boundingBox.bottomLeft.y,0.f,m_SpaceHeight) }) / m_NrOfCols, 0, m_NrOfRows - 1); j <= Elite::Clamp(PositionToIndex({ Elite::Clamp((boundingBox.bottomLeft + Elite::Vector2{ 0, boundingBox.height }).x,0.f,m_SpaceWidth), Elite::Clamp((boundingBox.bottomLeft + Elite::Vector2{ 0, boundingBox.height }).y,0.f,m_SpaceHeight) }) / m_NrOfCols, 0, m_NrOfRows - 1); j++)
	{
		for (size_t i = Elite::Clamp(PositionToIndex({ Elite::Clamp(boundingBox.bottomLeft.x,0.f,m_SpaceWidth), Elite::Clamp(boundingBox.bottomLeft.y,0.f,m_SpaceHeight) }) % m_NrOfCols, 0, m_NrOfCols - 1); i <= Elite::Clamp(PositionToIndex({ Elite::Clamp((boundingBox.bottomLeft + Elite::Vector2{boundingBox.width, 0}).x,0.f,m_SpaceWidth), Elite::Clamp((boundingBox.bottomLeft + Elite::Vector2{boundingBox.width, 0}).y,0.f,m_SpaceHeight) }) % m_NrOfCols, 0, m_NrOfCols - 1); i++)
		{
			DEBUGRENDERER2D->DrawPolygon(&Elite::Polygon{ m_Cells[j * m_NrOfCols + i].GetRectPoints() }, { 0,1,0 }, 0.4f);
		}
	}
}

int CellSpace::PositionToIndex(const Elite::Vector2 pos) const
{
	if (pos.x == m_SpaceWidth && pos.y == m_SpaceHeight)
	{
		return m_NrOfRows * m_NrOfCols - 1;
	}
	else if (pos.x == m_SpaceWidth)
	{
		return m_NrOfCols-1 + (int(pos.y / (m_SpaceHeight / m_NrOfRows)) * m_NrOfCols);
	}
	else if (pos.y == m_SpaceHeight)
	{
		return int(pos.x / (m_SpaceWidth / m_NrOfCols)) + (m_NrOfRows-1)*m_NrOfCols;
	}
	else
	{
		return int(pos.x/(m_SpaceWidth/m_NrOfCols)) + (int(pos.y / (m_SpaceHeight / m_NrOfRows))*m_NrOfCols);
	}
}