#include "pch.h"

/**/

#include "SpawnEngine.hpp"

import Engine.ACS;
import Engine.Assets;
import Engine.Common;
import Engine.Core;

using namespace hg;

namespace HeliogrimApi {
	using test_engine::SpawnEngine;

	TEST(SpawnEngine, PublishesActorModule) {
		const auto engine = make_uptr<SpawnEngine>();

		const auto actors = engine::ActorModule::get();
		ASSERT_TRUE(actors.has_value());
		EXPECT_EQ(std::addressof(actors.value()), engine->getActors().get());

		EXPECT_TRUE(engine::SerializationModule::get().has_value());
		EXPECT_FALSE(engine::Assets::get().has_value());
	}

	TEST(SpawnEngine, SequentialEnginesPublishTheirOwnModules) {
		{
			const auto first = make_uptr<SpawnEngine>();
			ASSERT_TRUE(engine::ActorModule::get().has_value());
		}

		EXPECT_FALSE(engine::ActorModule::get().has_value());
		EXPECT_FALSE(engine::SerializationModule::get().has_value());

		const auto second = make_uptr<SpawnEngine>();
		const auto actors = engine::ActorModule::get();
		ASSERT_TRUE(actors.has_value());
		EXPECT_EQ(std::addressof(actors.value()), second->getActors().get());
	}
}
