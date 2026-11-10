#include "pch.h"

/**/

#include <Engine.ACS/Traits.hpp>

using namespace hg;

namespace Acs {
	TEST(GuidGeneration, ConsecutiveGuidsDiffer) {
		const auto firstActor = generate_actor_guid();
		const auto secondActor = generate_actor_guid();
		EXPECT_NE(firstActor, secondActor);
		EXPECT_NE(firstActor, invalid_actor_guid);

		const auto firstComponent = generate_component_guid();
		const auto secondComponent = generate_component_guid();
		EXPECT_NE(firstComponent, secondComponent);
		EXPECT_NE(firstComponent, invalid_component_guid);
	}
}
