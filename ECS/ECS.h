#pragma once
#include <iostream>
#include <array>
#include <stack>
#include <vector>
#include <bitset>
#include <memory>

#include "Component.h"

#define OUT

constexpr std::size_t MAX_COMPONENT_COUNT = 32;

using EntityIndex = std::size_t;
using ComponentID = std::size_t;

class EntityManager
{
public:
	template<typename ... Ts>
	EntityManager(UsedComponents<Ts...> usedComponents)
	{
		SetupContainers<Ts...>();
	}

	~EntityManager()
	{
		CleanupContainers();
	}

	inline int EntityCount() const { return _firstUsableEntityIndex - _freeEntityIndices.size(); }
	EntityIndex CreateEntity();
	void DestroyEntity(EntityIndex entity);

	template <typename T> bool HasComponent(EntityIndex entity) const;
	template <typename T> void SetComponent(EntityIndex entity, T&& component);
	template <typename T> T GetComponent(EntityIndex entity) const;
	template <typename ... Ts> EntityIndex CreateEntityWithComponents(Ts... components);
	
	void GetEntities(const std::bitset<MAX_COMPONENT_COUNT>& filter,
					 OUT std::vector<EntityIndex>& entities) const;

private:
	bool TryReuseEntityIndex(OUT EntityIndex& entityIndex);
	void CreateContainersForNewEntity();
	void CleanupContainers();
	
	template <typename ... Ts> void SetupContainers();
	template <typename T> void SetupContainer();

	EntityIndex _firstUsableEntityIndex = 0;
	std::stack<EntityIndex> _freeEntityIndices;

	std::array<ComponentContainerBase*, MAX_COMPONENT_COUNT> _containers;
	std::vector<std::bitset<MAX_COMPONENT_COUNT>> _componentsByEntityIndex;
};

void EntityManager::GetEntities(const std::bitset<MAX_COMPONENT_COUNT>& filter,
								std::vector<EntityIndex>& entities) const
{
	for (EntityIndex i = 0; i < _firstUsableEntityIndex; ++i)
	{
		if ((_componentsByEntityIndex[i] & filter) == filter)
		{
			entities.push_back(i);
		}
	}
}

template <typename T> T EntityManager::GetComponent(EntityIndex entity) const
{
	static ComponentID id = GetComponentID<T>();

	const auto container = reinterpret_cast<ComponentContainer<T>*>(_containers[id]);
	return container->Get(entity);
}

template <typename T> bool EntityManager::HasComponent(EntityIndex entity) const
{
	static ComponentID id = GetComponentID<T>();
	return _componentsByEntityIndex[entity][id] == 1;
}

template <typename T> void EntityManager::SetComponent(EntityIndex entity, T&& component)
{
	static ComponentID id = GetComponentID<T>();
	
	auto container = reinterpret_cast<ComponentContainer<T>*>(_containers[id]);
	_componentsByEntityIndex[entity].set(id, true);
	container->Set(entity, std::forward<T>(component));
}

template <typename... Ts> void EntityManager::SetupContainers()
{
	_containers.fill(nullptr);
	auto _ = { (SetupContainer<Ts>(), 0)... };
}

template <typename T> void EntityManager::SetupContainer()
{
	auto id = GetComponentID<T>();
	_containers[id] = new ComponentContainer<T>();
}

template <typename... Ts> EntityIndex EntityManager::CreateEntityWithComponents(Ts... components)
{
	auto entity = CreateEntity();

	auto _ = { (SetComponent<Ts>(entity, std::forward<Ts>(components)), 0)... };

	return entity;
}

void EntityManager::CleanupContainers()
{
	for (auto& container : _containers)
	{
		delete container;
	}
}

EntityIndex EntityManager::CreateEntity()
{
	auto newEntity = _firstUsableEntityIndex;

	if (TryReuseEntityIndex(OUT newEntity))
	{
		SetComponent<EntityState>(newEntity, EntityState::Active);
		return newEntity;
	}

	CreateContainersForNewEntity();
	SetComponent<EntityState>(newEntity, EntityState::Active);
	_firstUsableEntityIndex++;
	return newEntity;
}

void EntityManager::CreateContainersForNewEntity()
{
	for (auto container : _containers)
	{
		if (container != nullptr)
		{
			container->AddNew();
		}
	}

	_componentsByEntityIndex.push_back(std::bitset<MAX_COMPONENT_COUNT>());
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

void EntityManager::DestroyEntity(EntityIndex index)
{
	_componentsByEntityIndex[index].reset();
	SetComponent<EntityState>(index, EntityState::Destroyed);
	_freeEntityIndices.push(index);
}
