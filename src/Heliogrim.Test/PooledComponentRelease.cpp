#include "pch.h"

/**/


#include "SpawnEngine.hpp"
#include "TestArchetypes.hpp"

import Engine.ACS;
import Engine.Common;

using namespace hg;

namespace HeliogrimApi {
	namespace {
		using test_engine::SpawnEngine;

		/* Drops a pool-acquired component through its default deleter, then tears the engine down. */
		template <class ComponentType_>
		void releaseThroughDefaultDeleter() {
			auto& pool = engine::acs::Registry::ComponentPool<ComponentType_>::getOrCreate();
			auto engine = make_uptr<SpawnEngine>();
			auto& registry = *engine->getActors()->getRegistry();
			const auto guid = ComponentGuid { generate_component_guid() };
			{
				auto component = registry.acquireActorComponent<ComponentType_>(
					guid,
					CachedActorPointer {},
					ptr<HierarchyComponent> { nullptr }
				);
				ASSERT_NE(component.get(), nullptr);
			}
			EXPECT_NE(pool.get(guid), nullptr);

			engine.reset();
			EXPECT_EQ(pool.get(guid), nullptr);
		}
	}

	TEST(PooledComponentRelease, DefaultDeleterFreesSlot) {
		releaseThroughDefaultDeleter<HierarchyComponent>();
	}

	/* A pooled type without its own meta class: its pool is keyed by its static type id, not its meta class. */
	TEST(PooledComponentRelease, DefaultDeleterFreesSlotOfInheritedMetaType) {
		releaseThroughDefaultDeleter<BodyStub>();
	}
}
