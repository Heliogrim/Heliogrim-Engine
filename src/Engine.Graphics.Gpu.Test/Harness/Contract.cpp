#include "Contract.hpp"

import std;

using namespace hg::test::gpu;
using namespace hg;

namespace {
	// Floor for the relative denominator so an expected 0 does not divide by zero.
	inline constexpr static f32 relFloor = 1e-6F;

	struct Tally {
		CheckResult result;

		void fail(const u32 x_, const u32 y_) {
			++result.failing;
			if (result.failingTexels.size() < listCap) {
				result.failingTexels.emplace_back(math::uivec2 { x_, y_ });
			}
		}

		[[nodiscard]] CheckResult done() {
			result.passed = result.failing == 0;
			return std::move(result);
		}
	};

	[[nodiscard]] Tally begin(String name_);

	template <typename Fn_>
	[[nodiscard]] Kernel perTexel(String name_, const u32 channels_, Fn_ err_);

	// Visits each adjacent pair along the axis; fn_ gets (x, y) of the first texel and the second texel's coordinates.
	template <typename Fn_>
	void forPairs(ref<const Raster> r_, const Axis axis_, Fn_ fn_);
}

Vector<CheckResult> hg::test::gpu::check(ref<const Contract> contract_, ref<const Raster> actual_, ref<const Raster> expected_) {
	// Kernels index both rasters with the actual extent, so a mismatch must fail before they run.
	const auto consistent = [](ref<const Raster> r_) { return r_.texels.size() == static_cast<size_t>(r_.extent.x) * r_.extent.y; };
	if (actual_.extent != expected_.extent || not consistent(actual_) || not consistent(expected_)) {
		return Vector<CheckResult> { CheckResult { "extent", false, 1, 0.F, {} } };
	}

	Vector<CheckResult> results {};
	results.reserve(contract_.checks.size());
	for (const auto& kernel : contract_.checks) {
		results.push_back(kernel(actual_, expected_));
	}
	return results;
}

Kernel hg::test::gpu::absEpsilon(const f32 eps_, const u32 channels_) {
	return perTexel(
		"absEpsilon",
		channels_,
		[eps_](const f32 a_, const f32 e_) {
			const auto err = std::abs(a_ - e_);
			return std::pair { err > eps_, err };
		}
	);
}

Kernel hg::test::gpu::relEpsilon(const f32 eps_, const u32 channels_) {
	return perTexel(
		"relEpsilon",
		channels_,
		[eps_](const f32 a_, const f32 e_) {
			const auto err = std::abs(a_ - e_) / std::max(std::abs(e_), relFloor);
			return std::pair { err > eps_, err };
		}
	);
}

Kernel hg::test::gpu::monotonic(const Axis axis_, const u32 channel_, const Order order_) {
	return [=](ref<const Raster> actual_, ref<const Raster> /*expected_*/) {
		auto t = begin("monotonic");
		forPairs(
			actual_,
			axis_,
			[&](const u32 x_, const u32 y_, const u32 nx_, const u32 ny_) {
				const auto a = actual_.at(x_, y_)[channel_];
				const auto b = actual_.at(nx_, ny_)[channel_];
				const auto ok = std::isfinite(a) && std::isfinite(b) && (order_ == Order::eIncreasing ? b >= a : b <= a);
				if (not ok) {
					t.fail(nx_, ny_);
				}
			}
		);
		t.result.measured = static_cast<f32>(t.result.failing);
		return t.done();
	};
}

Kernel hg::test::gpu::slopeEpsilon(const Axis axis_, const u32 channel_, const f32 eps_) {
	return [=](ref<const Raster> actual_, ref<const Raster> expected_) {
		auto t = begin("slopeEpsilon");
		forPairs(
			actual_,
			axis_,
			[&](const u32 x_, const u32 y_, const u32 nx_, const u32 ny_) {
				const auto da = actual_.at(nx_, ny_)[channel_] - actual_.at(x_, y_)[channel_];
				const auto de = expected_.at(nx_, ny_)[channel_] - expected_.at(x_, y_)[channel_];
				if (not std::isfinite(da) || not std::isfinite(de)) {
					t.fail(x_, y_);
					return;
				}

				const auto err = std::abs(da - de);
				t.result.measured = std::max(t.result.measured, err);
				if (err > eps_) {
					t.fail(x_, y_);
				}
			}
		);
		return t.done();
	};
}

/**/

namespace {
	[[nodiscard]] Tally begin(String name_) {
		return Tally { CheckResult { std::move(name_), true, 0, 0.F, {} } };
	}

	template <typename Fn_>
	[[nodiscard]] Kernel perTexel(String name_, const u32 channels_, Fn_ err_) {
		return [name = std::move(name_), channels_, err_](ref<const Raster> actual_, ref<const Raster> expected_) {
			auto t = begin(name);
			const auto n = std::min(channels_, 4u);
			for (u32 y = 0; y < actual_.extent.y; ++y) {
				for (u32 x = 0; x < actual_.extent.x; ++x) {
					const auto& a = actual_.at(x, y);
					const auto& e = expected_.at(x, y);
					bool bad = false;
					for (u32 c = 0; c < n; ++c) {
						if (not std::isfinite(a[c]) || not std::isfinite(e[c])) {
							bad = true;
							continue;
						}

						const auto [exceeds, err] = err_(a[c], e[c]);
						t.result.measured = std::max(t.result.measured, err);
						bad = bad || exceeds;
					}

					if (bad) {
						t.fail(x, y);
					}
				}
			}
			return t.done();
		};
	}

	template <typename Fn_>
	void forPairs(ref<const Raster> r_, const Axis axis_, Fn_ fn_) {
		const auto dx = axis_ == Axis::eX ? 1u : 0u;
		const auto dy = axis_ == Axis::eY ? 1u : 0u;
		for (u32 y = 0; y + dy < r_.extent.y; ++y) {
			for (u32 x = 0; x + dx < r_.extent.x; ++x) {
				fn_(x, y, x + dx, y + dy);
			}
		}
	}
}
