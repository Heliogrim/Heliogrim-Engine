#include "pch.h"

#include <chrono>
#include <cstddef>
#include <cstdio>
#include <memory>
#include <span>
#include <vector>
#include <Engine.Common/__warning.hpp>
#include <Engine.Common/Collection/Set.hpp>
#include <Heliogrim/Actor/ComponentRange.hpp>

using namespace hg;

/*
 * Disabled by default; run with `--gtest_also_run_disabled_tests --gtest_filter=*Bench*`.
 * Neither container dereferences a component, so the slot is plain aligned storage addressed as components.
 */
BEGIN_REGION_UNCHECKED_LLVL

namespace HeliogrimApi {
	namespace {
		constexpr u32 bench_align = 16u;
		constexpr u32 bench_stride = 128u; // roughly a HierarchyComponent rounded to the minimum alignment
		constexpr size_t bench_work = 1uLL << 24; // element visits per measurement

		template <typename Type_>
		void keep(const Type_& value_) noexcept {
			asm volatile("" : : "r,m"(value_) : "memory");
		}

		struct BenchActor {
			explicit BenchActor(const u32 count_) :
				blockSize(static_cast<u64>(count_) * bench_stride),
				storage(std::make_unique<std::byte[]>(blockSize + bench_align)),
				bitmap((blockSize / bench_align + 63u) / 64u, 0uLL) {
				auto* const raw = storage.get();
				slot = raw + ((bench_align - reinterpret_cast<uintptr_t>(raw) % bench_align) % bench_align);

				for (u32 i = 0u; i < count_; ++i) {
					const auto offset = static_cast<uintptr_t>(i) * bench_stride;
					offsets.push_back(offset);
					bitmap[offset / bench_align / 64u] |= 1uLL << (offset / bench_align % 64u);
					set.insert(component(i));
				}
			}

			[[nodiscard]] ptr<HierarchyComponent> component(const u32 index_) const noexcept {
				return reinterpret_cast<ptr<HierarchyComponent>>(slot + static_cast<size_t>(index_) * bench_stride);
			}

			[[nodiscard]] ComponentRange archetype() const noexcept {
				return ComponentRange::archetype(slot, offsets, bitmap.data(), static_cast<u32>(blockSize), bench_align);
			}

			u64 blockSize;
			std::unique_ptr<std::byte[]> storage;
			std::byte* slot = nullptr;
			std::vector<uintptr_t> offsets {};
			std::vector<u64> bitmap;
			CompactSet<ptr<HierarchyComponent>> set {};
		};

		template <typename Fn_>
		[[nodiscard]] double nanosPer(const size_t reps_, Fn_&& fn_) {
			for (size_t i = 0uLL; i < reps_ / 16uLL + 1uLL; ++i) {
				fn_(); // warm up
			}

			const auto start = std::chrono::steady_clock::now();
			for (size_t i = 0uLL; i < reps_; ++i) {
				fn_();
			}
			const auto stop = std::chrono::steady_clock::now();
			return std::chrono::duration<double, std::nano>(stop - start).count() / static_cast<double>(reps_);
		}

		template <typename Range_>
		[[nodiscard]] double traverse(const Range_& range_, const size_t reps_) {
			return nanosPer(
				reps_,
				[&range_] {
					uintptr_t sum = 0uLL;
					for (const auto* const component : range_) {
						sum += reinterpret_cast<uintptr_t>(component);
					}
					keep(sum);
				}
			);
		}

		/* Half hits (own components), half misses (another actor's components); ns per single lookup. */
		template <typename Contains_>
		[[nodiscard]] double lookup(cref<std::vector<ptr<HierarchyComponent>>> queries_, const size_t reps_, Contains_&& contains_) {
			const auto ns = nanosPer(
				reps_,
				[&] {
					size_t hits = 0uLL;
					for (const auto* const query : queries_) {
						hits += contains_(query) ? 1uLL : 0uLL;
					}
					keep(hits);
				}
			);
			return ns / static_cast<double>(queries_.size());
		}

		void runBench(const u32 count_) {
			const BenchActor actor { count_ };
			const BenchActor other { count_ };
			const auto archetype = actor.archetype();
			const auto dynamic = ComponentRange::dynamic(actor.set);

			std::vector<ptr<HierarchyComponent>> queries {};
			for (u32 i = 0u; i < count_; ++i) {
				queries.push_back(actor.component(i));
				queries.push_back(other.component(i));
			}

			const auto reps = bench_work / count_;
			const auto traverseArchetype = traverse(archetype, reps);
			const auto traverseDynamic = traverse(dynamic, reps);
			const auto traverseSet = traverse(actor.set, reps);

			const auto lookupReps = bench_work / queries.size();
			const auto containsArchetype = lookup(
				queries,
				lookupReps,
				[&archetype](const HierarchyComponent* q_) { return archetype.contains(q_); }
			);
			const auto containsDynamic = lookup(
				queries,
				lookupReps,
				[&dynamic](const HierarchyComponent* q_) { return dynamic.contains(q_); }
			);
			const auto containsSet = lookup(
				queries,
				lookupReps,
				[&actor](const HierarchyComponent* q_) {
					return actor.set.contains(const_cast<ptr<HierarchyComponent>>(q_));
				}
			);

			std::printf(
				"[bench] n=%2u traverse ns: range(archetype)=%8.2f range(dynamic)=%8.2f CompactSet=%8.2f\n",
				count_,
				traverseArchetype,
				traverseDynamic,
				traverseSet
			);
			std::printf(
				"[bench] n=%2u contains ns: range(archetype)=%8.2f range(dynamic)=%8.2f CompactSet=%8.2f\n",
				count_,
				containsArchetype,
				containsDynamic,
				containsSet
			);
		}
	}

	TEST(ComponentRangeBench, DISABLED_Size1) {
		runBench(1u);
	}

	TEST(ComponentRangeBench, DISABLED_Size4) {
		runBench(4u);
	}

	TEST(ComponentRangeBench, DISABLED_Size16) {
		runBench(16u);
	}

	TEST(ComponentRangeBench, DISABLED_Size64) {
		runBench(64u);
	}
}

END_REGION_UNCHECKED_LLVL
