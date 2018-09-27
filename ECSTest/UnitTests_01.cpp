#include <string>
#include "stdafx.h"
#include "CppUnitTest.h"
#include "../ECS/ECS.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace ECSTest
{
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
	};
}