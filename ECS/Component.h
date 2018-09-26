#pragma once

class ComponentBase
{
};

enum class EntityState
{
	Unknown,
	Active,
	Destroyed
};

std::size_t GetNextComponentID()
{
	static std::size_t compID {0};
	return compID++;
}


template <typename T>
std::size_t GetComponentID()
{
	static std::size_t id = GetNextComponentID();
	return id;
};
