#pragma once
#include <stack>
#include <unordered_map>
#include <vector>
#include <bitset>
#include "Component.h"

#define OUT

constexpr std::size_t MAX_COMPONENT_COUNT = 32;

using EntityIndex = std::size_t;
using ComponentID = std::size_t;

class EntityManager
{
public:
	int EntityCount() const
	{
		return _firstUsableEntityIndex - _freeEntityIndices.size();
	}

	EntityIndex CreateEntity();

	template <typename T> bool HasComponent(EntityIndex entity) const;

private:

	bool TryReuseEntityIndex(OUT EntityIndex& entityIndex);
	void CreateContainersForNewEntity();

	EntityIndex _firstUsableEntityIndex = 0;
	std::stack<EntityIndex> _freeEntityIndices;
	std::unordered_map<ComponentID, std::vector<ComponentBase*>> _containers;
	std::unordered_map<EntityIndex, std::bitset<MAX_COMPONENT_COUNT>> _componentsByEntityIndex;
};

template <typename T>
bool EntityManager::HasComponent(EntityIndex entity) const
{
	static ComponentID id = GetComponentID<T>();
	return _componentsByEntityIndex.find(entity)->second[id] == 1;
}

EntityIndex EntityManager::CreateEntity()
{
	auto newEntity = _firstUsableEntityIndex;

	if (TryReuseEntityIndex(OUT newEntity))
	{
		return newEntity;
	}

	CreateContainersForNewEntity();
	_firstUsableEntityIndex++;
	return newEntity;
}

void EntityManager::CreateContainersForNewEntity()
{
	for (auto& container : _containers)
	{
		container.second.push_back(nullptr);
	}

	_componentsByEntityIndex[_firstUsableEntityIndex] = std::bitset<MAX_COMPONENT_COUNT>();
}

bool EntityManager::TryReuseEntityIndex(OUT EntityIndex& entityIndex)
{
	if (_freeEntityIndices.size() > 0)
	{
		entityIndex = _freeEntityIndices.top();
		_freeEntityIndices.pop();
		return true;
	}

	return false;
}
