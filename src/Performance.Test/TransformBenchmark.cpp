#include "pch.h"

/**/

#include <chrono>
#include <cstdio>
#include <vector>
#include <Engine.ACS/Traits.hpp>
#include <Engine.Common/Move.hpp>
#include <Engine.Common/Math/Rotator.hpp>
#include <Engine.Common/Math/Transform.hpp>
#include <Heliogrim/Component/HierarchyComponent.hpp>

using namespace hg;

/**
 * Micro benchmarks for the location and rotation hot path.
 *
 * All benchmarks are DISABLED by default and print `ns/op`; run them explicitly with
 *	`--gtest_also_run_disabled_tests --gtest_filter=TransformBenchmark.*`
 *
 * Note: Only a debug configuration is built by default, so absolute numbers are indicative; compare before/after only.
 */
namespace Performance {
	namespace {
		constexpr int iterations = 200'000;

		/**
		 * Prevents the optimizer from dropping the measured work.
		 */
		template <typename Type_>
		inline void consume(const Type_& value_) {
			static volatile float sink;
			sink = *reinterpret_cast<const volatile float*>(&value_);
		}

		template <typename Fn_>
		[[nodiscard]] double nanosPerOp(Fn_&& fn_) {

			// Warm-up
			for (int i = 0; i < iterations / 10; ++i) {
				fn_(i);
			}

			const auto start = std::chrono::steady_clock::now();
			for (int i = 0; i < iterations; ++i) {
				fn_(i);
			}
			const auto end = std::chrono::steady_clock::now();

			return static_cast<double>(std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count()) /
				static_cast<double>(iterations);
		}

		void report(const char* name_, const double nanos_) {
			std::printf("[ BENCH    ] %-48s %10.1f ns/op\n", name_, nanos_);
		}

		/**
		 * Builds a chain of `depth_ + 1` components where each level carries a combined-axis rotation and a translation.
		 */
		struct Chain {
			std::vector<HierarchyComponent> components;

			explicit Chain(const size_t depth_) {
				components.reserve(depth_ + 1);
				for (size_t i = 0; i <= depth_; ++i) {
					components.emplace_back(
						generate_component_guid(),
						CachedActorPointer { .guid = invalid_actor_guid, .cached = nullptr },
						i == 0 ? nullptr : &components.back()
					);

					const auto fi = static_cast<float>(i);
					components.back().getLocalTransform().setLocationAndRotator(
						math::Location { 1.F + fi, 2.F, 3.F - fi },
						math::Rotator { 0.1F + 0.05F * fi, 0.2F, -0.1F }
					);
				}
			}

			[[nodiscard]] ref<HierarchyComponent> leaf() noexcept {
				return components.back();
			}
		};
	}

	TEST(TransformBenchmark, DISABLED_RotatorRotated) {
		const auto rot = math::Rotator { 0.3F, 0.7F, -0.2F };
		auto vec = math::fvec3 { 1.F, 2.F, 3.F };

		report("Rotator::rotated", nanosPerOp([&](const int i) {
			vec.x = static_cast<float>(i & 7);
			const auto out = rot.rotated(vec);
			consume(out);
		}));
	}

	TEST(TransformBenchmark, DISABLED_RotatorInversed) {
		const auto rot = math::Rotator { 0.3F, 0.7F, -0.2F };
		auto vec = math::fvec3 { 1.F, 2.F, 3.F };

		report("Rotator::inversed", nanosPerOp([&](const int i) {
			vec.x = static_cast<float>(i & 7);
			const auto out = rot.inversed(vec);
			consume(out);
		}));
	}

	TEST(TransformBenchmark, DISABLED_TransformAsMatrix) {
		auto transform = math::Transform { math::Location { 1.F, 2.F, 3.F }, math::Rotator { 0.3F, 0.7F, -0.2F }, math::vec3 { 2.F, 1.F, 0.5F } };

		report("Transform::asMatrix", nanosPerOp([&](const int i) {
			transform.setLocation(math::Location { static_cast<float>(i & 7), 2.F, 3.F });
			const auto out = transform.asMatrix();
			consume(out);
		}));
	}

	TEST(TransformBenchmark, DISABLED_HierarchyUniverseMatrixDepth1) {
		auto chain = Chain { 1 };
		report("HierarchyComponent::getUniverseMatrix depth 1", nanosPerOp([&](int) {
			const auto out = chain.leaf().getUniverseMatrix();
			consume(out);
		}));
	}

	TEST(TransformBenchmark, DISABLED_HierarchyUniverseMatrixDepth4) {
		auto chain = Chain { 4 };
		report("HierarchyComponent::getUniverseMatrix depth 4", nanosPerOp([&](int) {
			const auto out = chain.leaf().getUniverseMatrix();
			consume(out);
		}));
	}

	TEST(TransformBenchmark, DISABLED_HierarchyUniverseMatrixDepth8) {
		auto chain = Chain { 8 };
		report("HierarchyComponent::getUniverseMatrix depth 8", nanosPerOp([&](int) {
			const auto out = chain.leaf().getUniverseMatrix();
			consume(out);
		}));
	}

	TEST(TransformBenchmark, DISABLED_HierarchyUniverseLocationAndRotatorDepth4) {
		auto chain = Chain { 4 };
		report("HierarchyComponent::getUniverse{Location,Rotator} depth 4", nanosPerOp([&](int) {
			const auto loc = chain.leaf().getUniverseLocation();
			const auto rot = chain.leaf().getUniverseRotator();
			consume(loc);
			consume(rot);
		}));
	}

	TEST(TransformBenchmark, DISABLED_HierarchySetRelativeDepth4) {
		auto chain = Chain { 4 };
		report("HierarchyComponent::setRelativeLocationAndRotator depth 4", nanosPerOp([&](const int i) {
			chain.leaf().setRelativeLocationAndRotator(
				math::Location { static_cast<float>(i & 7), 1.F, 2.F },
				math::Rotator { 0.1F, 0.2F, 0.3F }
			);
			consume(chain.leaf().getLocalTransform());
		}));
	}
}
