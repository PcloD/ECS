#pragma once

enum class EntityState
{
	Unknown,
	Active,
	Destroyed
};

std::size_t GetNextComponentID()
{
	static std::size_t compID{ 0 };
	return compID++;
}

template <typename T> std::size_t GetComponentID()
{
	static std::size_t id = GetNextComponentID();
	return id;
};

//TODO naming
template <typename... Ts> class UsedComponents { };

class ComponentContainerBase
{
public:
	virtual void AddNew() = 0;
};

template <typename T>
class ComponentContainer : public ComponentContainerBase
{
public:
	void AddNew() override;
	void Set(std::size_t index, T&& value);
	const T& Get(std::size_t index) const;

private:
	std::vector<T> _components;
};

template <typename T>
void ComponentContainer<T>::AddNew()
{
	_components.push_back(T{});
}

template <typename T>
void ComponentContainer<T>::Set(std::size_t index, T&& value)
{
	_components[index] = value;
}

template <typename T>
const T& ComponentContainer<T>::Get(std::size_t index) const
{
	return _components[index];
}






