#include "pch.h"

/**/

import Engine.ACS;
import Engine.Serialization;
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
