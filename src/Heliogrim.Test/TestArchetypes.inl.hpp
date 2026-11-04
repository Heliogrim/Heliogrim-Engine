#pragma once

#include "TestArchetypes.hpp"

namespace HeliogrimApi {
	// Deducing this: `get` resolves against the concrete archetype, so an Override that moves "camera" still hits it.
	void TestOverridable::tick(this auto& self) {
		++self.template get<"camera">().ticks;
	}

	void TestOverrider::tick(this auto& self) {
		self.TestOverridable::tick();
		self.template get<"camera">().zoom[0] = 42uLL;
	}
}
