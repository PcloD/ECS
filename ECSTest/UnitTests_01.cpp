#include "stdafx.h"

#include <string>
#include <iostream>
#include <chrono>
#include <functional>
#include <future>

#include "CppUnitTest.h"
#include "../ECS/ECS.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace ECSTest
{
	constexpr std::size_t MANY = 1000;

	struct TestComponent
	{
		float x, y, z;
	};

	TEST_CLASS(UnitTest01)
	{
	public:
		TEST_METHOD(EntityCountTest)
		{
			UsedComponents<EntityState> usedComponents;

			EntityManager manager(usedComponents);
			Assert::IsTrue(manager.EntityCount() == 0, std::to_wstring(manager.EntityCount()).c_str());

			auto entity = manager.CreateEntity();
			Assert::IsTrue(manager.EntityCount() == 1);
			Assert::IsTrue(entity == 0);
			Assert::IsTrue(manager.HasComponent<EntityState>(entity));
			Assert::IsTrue(manager.GetComponent<EntityState>(entity) == EntityState::Active);

			auto other = manager.CreateEntity();
			Assert::IsTrue(manager.EntityCount() == 2);
			Assert::IsTrue(other == 1);

			manager.DestroyEntity(entity);
			Assert::IsTrue(manager.EntityCount() == 1);

			entity = manager.CreateEntity();
			Assert::IsTrue(manager.EntityCount() == 2);
			Assert::IsTrue(entity == 0);
			Assert::IsTrue(manager.GetComponent<EntityState>(entity) == EntityState::Active);
		}

		TEST_METHOD(MultiComponentsTest)
		{
			UsedComponents<EntityState, int> usedComponents;
			EntityManager manager(usedComponents);
			Assert::IsTrue(manager.EntityCount() == 0);

			auto entity = manager.CreateEntityWithComponents<int>(123);

			Assert::IsTrue(manager.EntityCount() == 1);
			Assert::IsTrue(manager.HasComponent<EntityState>(entity));
			Assert::IsTrue(manager.HasComponent<int>(entity));

			auto other = manager.CreateEntity();

			Assert::IsTrue(manager.EntityCount() == 2);
			Assert::IsTrue(manager.HasComponent<EntityState>(other));
			Assert::IsFalse(manager.HasComponent<int>(other));

			auto third = manager.CreateEntityWithComponents<int>(99);

			Assert::IsTrue(manager.EntityCount() == 3);
			Assert::IsTrue(manager.GetComponent<int>(third) == 99);
		}

		TEST_METHOD(EntityReuseWithDifferentComponents)
		{
			using IntVec = std::vector<int>;
			UsedComponents<EntityState, int, float, IntVec> usedComponents;
			EntityManager manager(usedComponents);
			Assert::IsTrue(manager.EntityCount() == 0);

			auto first = manager.CreateEntityWithComponents<int, IntVec>(123, IntVec(1, 10));
			Assert::IsTrue(manager.EntityCount() == 1);
			Assert::IsTrue(manager.HasComponent<int>(first));
			Assert::IsTrue(manager.HasComponent<IntVec>(first));

			auto second = manager.CreateEntityWithComponents<float>(111.0f);
			Assert::IsTrue(manager.EntityCount() == 2);
			Assert::IsTrue(manager.HasComponent<float>(second));
			Assert::IsFalse(manager.HasComponent<int>(second));
			Assert::IsFalse(manager.HasComponent<IntVec>(second));

			manager.DestroyEntity(second);
			Assert::IsTrue(manager.EntityCount() == 1);
			
			manager.DestroyEntity(first);
			Assert::IsTrue(manager.EntityCount() == 0);

			second = manager.CreateEntityWithComponents<float>(99.0f);
			Assert::IsTrue(manager.EntityCount() == 1);
			Assert::IsTrue(second == 0);
			Assert::IsTrue(manager.HasComponent<float>(second));
			Assert::IsFalse(manager.HasComponent<IntVec>(second));
			Assert::IsFalse(manager.HasComponent<int>(second));

			first = manager.CreateEntityWithComponents<IntVec>(IntVec(1, 1));
			Assert::IsTrue(manager.EntityCount() == 2);
			Assert::IsTrue(first == 1);
			Assert::IsTrue(manager.HasComponent<IntVec>(first));
			Assert::IsFalse(manager.HasComponent<float>(first));
			Assert::IsFalse(manager.HasComponent<int>(first));
		}

		TEST_METHOD(GetOnlyLiveEntitiesForComponents)
		{
			UsedComponents<EntityState, int, TestComponent> usedComponents;
			EntityManager manager(usedComponents);
			Assert::IsTrue(manager.EntityCount() == 0);

			for (int i = 0; i < 1000; ++i)
			{
				manager.CreateEntityWithComponents<int, TestComponent>(i, std::move(TestComponent{}));
			}

			Assert::IsTrue(manager.EntityCount() == 1000);

			std::bitset<MAX_COMPONENT_COUNT> filter;

			filter.reset();
			filter.set(GetComponentID<EntityState>(), true);
			filter.set(GetComponentID<int>(), true);

			std::vector<EntityIndex> entities;

			std::function<void()> doFilter = [&]() -> void { manager.GetEntities(filter, OUT entities); };
			Measure(doFilter, "Filter: ");

			Assert::IsTrue(entities.size() == 1000);

			for (EntityIndex i = 0; i < 1000; ++i)
			{
				if (i % 3 == 0)
				{
					manager.DestroyEntity(i);
				}
			}

			Measure(doFilter, "Filter only live ones: ");

			Assert::IsTrue(entities.size() == 666, std::to_wstring(entities.size()).c_str());
		}

		TEST_METHOD(GetEntitiesForComponents)
		{
			//TODO: _always_ have EntityState, even if not explicitly specified
			UsedComponents<EntityState, int, TestComponent> usedComponents;
			EntityManager manager(usedComponents);
			Assert::IsTrue(manager.EntityCount() == 0);

			auto start = std::chrono::high_resolution_clock::now();

			for (int i = 0; i < 1000; ++i)
			{
				if (i % 2 == 0)
				{
					manager.CreateEntityWithComponents<int, TestComponent>(i, TestComponent{});
				}
				else
				{
					manager.CreateEntityWithComponents<int>(i);
				}
			}

			auto end = std::chrono::high_resolution_clock::now();
			std::chrono::duration<double> elapsed = end - start;
			std::string msg = "Creation: " + std::to_string(elapsed.count());
			Logger::WriteMessage(msg.c_str());

			Assert::IsTrue(manager.EntityCount() == 1000);

			//TODO: Extract this nicely
			std::bitset<MAX_COMPONENT_COUNT> filter;

			filter.reset();

			filter.set(GetComponentID<EntityState>(), true);
			filter.set(GetComponentID<int>(), true);
			filter.set(GetComponentID<TestComponent>(), true);
			
			std::vector<EntityIndex> entities;

			start = std::chrono::high_resolution_clock::now();

			manager.GetEntities(filter, OUT entities);

			end = std::chrono::high_resolution_clock::now();
			elapsed = end - start;

			msg = "\nFiltering: " + std::to_string(elapsed.count());
			Logger::WriteMessage(msg.c_str());

			Assert::IsTrue(entities.size() == 500, std::to_wstring(entities.size()).c_str());
		}

		TEST_METHOD(DoSomeWork)
		{
			UsedComponents<EntityState, int> usedComponents;
			EntityManager manager(usedComponents);
			for (int i = 0; i < MANY; ++i)
			{
				manager.CreateEntityWithComponents<int>(i);
			}

			Assert::IsTrue(manager.EntityCount() == MANY);

			EntityFilter filter;
			filter.set(GetComponentID<EntityState>(), true);
			filter.set(GetComponentID<int>(), true);
			
			std::vector<EntityIndex> entities;

			manager.GetEntities(filter, OUT entities);

			auto intContainer = manager.GetContainer<int>();

			std::function<void()> setEntities = [&]
			{
				for (const auto& entity : entities)
				{
					auto current = intContainer->Get(entity);
					intContainer->Set(entity, current + 1);
				}
			};

			Measure(setEntities, "Set <int> on MANY entities: ");

			for (int i = 0; i < MANY; ++i)
			{
				Assert::IsTrue(manager.GetComponent<int>(entities[i]) == i + 1);
			}
		}

		TEST_METHOD(DoSomeParallelWork)
		{
			UsedComponents<EntityState, int> usedComponents;
			EntityManager manager(usedComponents);
			for (int i = 0; i < MANY; ++i)
			{
				manager.CreateEntityWithComponents<int>(i);
			}

			Assert::IsTrue(manager.EntityCount() == MANY);

			EntityFilter filter;
			filter.set(GetComponentID<EntityState>(), true);
			filter.set(GetComponentID<int>(), true);

			std::vector<EntityIndex> entities;

			manager.GetEntities(filter, OUT entities);

			auto intContainer = manager.GetContainer<int>();

			auto increment = [&](EntityIndex from, EntityIndex until)
			{
				for (EntityIndex i = from; i < until; ++i)
				{
					auto entity = entities[i];
					auto current = intContainer->Get(entity);
					intContainer->Set(entity, current + 1);
				}
			};

			std::function<void()> doParallelWork = [&]
			{
				auto size = entities.size();
				auto future1 = std::async(std::launch::async, increment, 0, size / 2);
				auto future2 = std::async(std::launch::async, increment, size / 2, size);

				future1.get();
				future2.get();
			};

			Measure(doParallelWork, "Set <int> on MANY entities in parallel: ");

			for (int i = 0; i < MANY; ++i)
			{
				Assert::IsTrue(manager.GetComponent<int>(entities[i]) == i + 1);
			}
		}

	private:
		void Measure(std::function<void()> func, const std::string& name)
		{
			auto start = std::chrono::high_resolution_clock::now();
			func();
			auto end = std::chrono::high_resolution_clock::now();
			std::chrono::duration<double> elapsed = end - start;

			auto msg = name + ": " + std::to_string(elapsed.count()) + " sec\n";
			Logger::WriteMessage(msg.c_str());
		}
	};
}