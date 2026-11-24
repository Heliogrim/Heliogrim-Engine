#include "pch.h"

/**/


#include "SpawnEngine.hpp"

import Engine.Common;
import Engine.ACS;
import Heliogrim.Actors;
import Engine.Graphics.Components;

using namespace hg;

namespace HeliogrimApi {
	/* No actor pool is registered up front, as in a headless engine without an editor. */

	TEST(TypedCreateActor, PlainActor) {
		auto engine = make_uptr<test_engine::SpawnEngine>();
		auto actor = CreateActor<Actor>();
		ASSERT_NE(actor.get(), nullptr);
		EXPECT_TRUE(engine->getActors()->getRegistry()->getOrCreateActorPool<Actor>()->get(actor->guid()) != nullptr);
	}

	TEST(TypedCreateActor, CameraActorIsAutoInitialized) {
		auto engine = make_uptr<test_engine::SpawnEngine>();
		auto actor = CreateActor<CameraActor>();
		ASSERT_NE(actor.get(), nullptr);
		EXPECT_NE(actor->getCameraComponent(), nullptr);
	}

	TEST(TypedCreateActor, PointLightActorIsAutoInitialized) {
		auto engine = make_uptr<test_engine::SpawnEngine>();
		auto actor = CreateActor<PointLightActor>();
		ASSERT_NE(actor.get(), nullptr);
		EXPECT_NE(actor->getPointLightComponent(), nullptr);
	}
}
