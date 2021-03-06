#pragma once

enum
{
	invalid_node_index = -1
};


enum class TerrainType : int
{
	Unknown = 0,
	Ground = 1,
	Hill = 2,
	Mud = 3,
	Forest = 4,
	// Node's with a value of over 200 000 are always isolated
	Water = 200001,
	Wall = 200002
};