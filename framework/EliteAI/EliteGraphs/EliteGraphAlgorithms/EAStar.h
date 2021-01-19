#pragma once

namespace Elite
{
	template <class T_NodeType, class T_ConnectionType>
	class AStar
	{
	public:
		AStar(IGraph<T_NodeType, T_ConnectionType>* pGraph, Heuristic hFunction);

		// stores the optimal connection to a node and its total costs related to the start and end node of the path
		struct NodeRecord
		{
			T_NodeType* pNode = nullptr;
			T_ConnectionType* pConnection = nullptr;
			float costSoFar = 0.f; // accumulated g-costs of all the connections leading up to this one
			float estimatedTotalCost = 0.f; // f-cost (= costSoFar + h-cost)

			bool operator==(const NodeRecord& other) const
			{
				return pNode == other.pNode
					&& pConnection == other.pConnection
					&& costSoFar == other.costSoFar
					&& estimatedTotalCost == other.estimatedTotalCost;
			};

			bool operator<(const NodeRecord& other) const
			{
				return estimatedTotalCost < other.estimatedTotalCost;
			};
		};

		std::vector<T_NodeType*> FindPath(T_NodeType* pStartNode, T_NodeType* pDestinationNode);

	private:
		float GetHeuristicCost(T_NodeType* pStartNode, T_NodeType* pEndNode) const;

		IGraph<T_NodeType, T_ConnectionType>* m_pGraph;
		Heuristic m_HeuristicFunction;
	};

	template <class T_NodeType, class T_ConnectionType>
	AStar<T_NodeType, T_ConnectionType>::AStar(IGraph<T_NodeType, T_ConnectionType>* pGraph, Heuristic hFunction)
		: m_pGraph(pGraph)
		, m_HeuristicFunction(hFunction)
	{
	}

	template <class T_NodeType, class T_ConnectionType>
	std::vector<T_NodeType*> AStar<T_NodeType, T_ConnectionType>::FindPath(T_NodeType* pStartNode, T_NodeType* pGoalNode)
	{
		vector<T_NodeType*> path;
		vector<NodeRecord> openList;
		vector<NodeRecord> closedList;

		NodeRecord startRecord{};
		startRecord.pNode = pStartNode;
		startRecord.estimatedTotalCost = GetHeuristicCost(pStartNode, pGoalNode);

		openList.push_back(startRecord);
		
		bool skip{};
		NodeRecord currentRecord{};
		while (openList.size() != 0)
		{
			currentRecord = {};
			for (NodeRecord record : openList)
			{
				if (currentRecord.pNode == nullptr || currentRecord.estimatedTotalCost > record.estimatedTotalCost)
				{
					currentRecord = record;
				}
			}
			for (T_ConnectionType* connection : m_pGraph->GetNodeConnections(currentRecord.pNode->GetIndex()))
			{
				skip = false;
				if (m_pGraph->GetNode(connection->GetTo()) == pGoalNode)
				{
					goto finished;
				}

				NodeRecord newRecord{};
				newRecord.pNode = m_pGraph->GetNode(connection->GetTo());
				newRecord.costSoFar = currentRecord.costSoFar + connection->GetCost();
				newRecord.estimatedTotalCost = newRecord.costSoFar + GetHeuristicCost(newRecord.pNode, pGoalNode);
				newRecord.pConnection = connection;


				for (NodeRecord record : closedList)
				{
					if (m_pGraph->GetNode(connection->GetTo()) == record.pNode && newRecord.costSoFar < record.costSoFar)
					{
						closedList.erase(std::remove(closedList.begin(), closedList.end(), record),closedList.end());
					}
					else if (m_pGraph->GetNode(connection->GetTo()) == record.pNode)
					{
						skip = true;
						break;
					}
				}
				for (NodeRecord record : openList)
				{
					if (m_pGraph->GetNode(connection->GetTo()) == record.pNode && /*record.costSoFar < gCost*/ newRecord.costSoFar < record.costSoFar)
					{
						openList.erase(std::remove(openList.begin(), openList.end(), record),openList.end());
					}
					else if(m_pGraph->GetNode(connection->GetTo()) == record.pNode)
					{
						skip = true;
						break;
					}
				}
				if (!skip)
				{
					openList.push_back(newRecord);
				}
			}
			closedList.push_back(currentRecord);
			openList.erase(std::remove(openList.begin(), openList.end(), currentRecord), openList.end());
		
		}

		finished:

		path.push_back(pGoalNode);

		while (currentRecord.pNode != pStartNode)
		{
			path.push_back(currentRecord.pNode);
			for (NodeRecord record : closedList)
			{
				if (m_pGraph->GetNode(currentRecord.pConnection->GetFrom()) == record.pNode)
				{
					currentRecord = record;
					break;
				}
			}
		}
		path.push_back(pStartNode);
		reverse(path.begin(), path.end());
		return path;

	}

	template <class T_NodeType, class T_ConnectionType>
	float Elite::AStar<T_NodeType, T_ConnectionType>::GetHeuristicCost(T_NodeType* pStartNode, T_NodeType* pEndNode) const
	{
		Vector2 toDestination = m_pGraph->GetNodePos(pEndNode) - m_pGraph->GetNodePos(pStartNode);
		return m_HeuristicFunction(abs(toDestination.x), abs(toDestination.y));
	}
}