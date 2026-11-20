#pragma once

#include "Raster.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::test::gpu {
	// Half-ulp scale of f32 around 1.0 times the number of ops in a short function.
	inline constexpr static f32 defaultAbsEpsilon = 1e-5F;

	// Maximum number of failing texels (or window origins) listed in a result.
	inline constexpr static u32 listCap = 32;

	enum class Axis : u8 {
		eX,
		eY
	};

	enum class Order : u8 {
		eIncreasing,
		eDecreasing
	};

	struct CheckResult {
		String name;
		bool passed;
		u32 failing;
		f32 measured;
		Vector<math::uivec2> failingTexels;
	};

	using Kernel = std::function<CheckResult(ref<const Raster> actual_, ref<const Raster> expected_)>;

	struct Contract {
		Vector<Kernel> checks;
	};

	[[nodiscard]] Vector<CheckResult> check(ref<const Contract> contract_, ref<const Raster> actual_, ref<const Raster> expected_);

	[[nodiscard]] Kernel absEpsilon(const f32 eps_, const u32 channels_ = 4);

	[[nodiscard]] Kernel relEpsilon(const f32 eps_, const u32 channels_ = 4);

	[[nodiscard]] Kernel monotonic(const Axis axis_, const u32 channel_, const Order order_);

	[[nodiscard]] Kernel slopeEpsilon(const Axis axis_, const u32 channel_, const f32 eps_);
}
