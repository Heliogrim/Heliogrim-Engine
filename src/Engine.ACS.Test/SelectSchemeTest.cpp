#include "pch.h"

/**/

#include <Engine.ACS/Actor/Actor.hpp>
#include <Engine.ACS/Component/LogicComponent.hpp>
#include <Engine.Serialization/Scheme/Selector.hpp>

import std;

using namespace hg::engine::serialization;
using namespace hg;

namespace Acs {
	namespace {
		class TestComponent : public LogicComponent {};
	}

	static_assert(std::same_as<SelectScheme<Actor>::default_type, ActorScheme<Actor>>);
	static_assert(std::same_as<SelectScheme<TestComponent>::default_type, ComponentScheme<TestComponent>>);
	static_assert(std::same_as<SelectScheme<int>::default_type, DefaultScheme<int>>);

	TEST(SelectScheme, ResolvesActorComponentAndDefaultSchemes) {
		SUCCEED();
	}
}
