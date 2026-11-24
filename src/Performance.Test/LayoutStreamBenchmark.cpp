#include "pch.h"

/**/
#include <Engine.ACS/Archetype/ArchetypeDefinition.inl.hpp>
#include <Engine.ACS/Archetype/ArchetypeLayout.inl.hpp>
#ifdef _WIN32
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif
import Engine.ACS;
import Engine.Common;
import Engine.Serialization;

/**
 * Archetype layout stream (dumpLayout / loadLayout) against plain memcpy / memmove of the same bytes.
 *
 * DISABLED by default and skipped on Debug (NDEBUG undefined); run explicitly with
 *	`--gtest_also_run_disabled_tests --gtest_filter=LayoutStreamBenchmark.*`
 *
 * Baseline: one memcpy (or memmove) of the slot payload per slot, between the same stream buffer and the same slot
 * memory. Both stubs keep their streamed members contiguous and in stream order, so that is a single call per slot.
 */
namespace HeliogrimApi {
	using namespace ::hg::engine::acs;
	using namespace ::hg;

	enum class BenchMode : u8 { eA, eB, eC, eD };

	struct NumStub : HierarchyComponent {
		using HierarchyComponent::HierarchyComponent;
		inline constexpr static ComponentTypeId typeId { "BenchNumStub"_typeId };

		u64 a = 0uLL;
		u32 b = 0u;
		u32 c = 0u;
		f32 d[120] {};
	};

	struct MixStub : HierarchyComponent {
		using HierarchyComponent::HierarchyComponent;
		inline constexpr static ComponentTypeId typeId { "BenchMixStub"_typeId };

		u64 a = 0uLL;
		u32 b = 0u;
		u32 c = 0u;
		f32 d[120] {};
		bool armed = false;
		BenchMode mode = BenchMode::eA;
	};

	class BenchNum :
		public InheritArchetype<BenchNum, Actor> {
	public:
		using this_type = BenchNum;
		inline constexpr static type_id typeId { "BenchNum"_typeId };

		using layout = ActorLayout<Slot<"data", NumStub>>;

	public:
		explicit BenchNum(ref<const ActorInitializer> initializer_) :
			InheritArchetype(initializer_) {}
	};

	class BenchMix :
		public InheritArchetype<BenchMix, Actor> {
	public:
		using this_type = BenchMix;
		inline constexpr static type_id typeId { "BenchMix"_typeId };

		using layout = ActorLayout<Slot<"data", MixStub>>;

	public:
		explicit BenchMix(ref<const ActorInitializer> initializer_) :
			InheritArchetype(initializer_) {}
	};
}

template <>
class hg::engine::serialization::access::Layout<HeliogrimApi::NumStub> {
public:
	inline constexpr static std::array fields {
		HG_LAYOUT_RAW(HeliogrimApi::NumStub, a, ::hg::u64),
		HG_LAYOUT_RAW(HeliogrimApi::NumStub, b, ::hg::u32),
		HG_LAYOUT_RAW(HeliogrimApi::NumStub, c, ::hg::u32),
		HG_LAYOUT_RAW(HeliogrimApi::NumStub, d, ::hg::f32[120])
	};
};

template <>
class hg::engine::serialization::access::Layout<HeliogrimApi::MixStub> {
public:
	inline constexpr static std::array fields {
		HG_LAYOUT_RAW(HeliogrimApi::MixStub, a, ::hg::u64),
		HG_LAYOUT_RAW(HeliogrimApi::MixStub, b, ::hg::u32),
		HG_LAYOUT_RAW(HeliogrimApi::MixStub, c, ::hg::u32),
		HG_LAYOUT_RAW(HeliogrimApi::MixStub, d, ::hg::f32[120]),
		HG_LAYOUT_MASK(HeliogrimApi::MixStub, armed, 0x01u),
		HG_LAYOUT_MASK(HeliogrimApi::MixStub, mode, 0x03u)
	};
};

/* Structured schemes are never used here; the archetype ops reference them. */
using namespace hg;
using namespace hg::engine::serialization;

#define HELIOGRIM_BENCH_NOOP_STRUCTURE(Type_) \
	template <> \
	void access::Structure<HeliogrimApi::Type_>::serialize(ref<const HeliogrimApi::Type_>, mref<StructScopedSlot>) {} \
	template <> \
	void access::Structure<HeliogrimApi::Type_>::hydrate(ref<const StructScopedSlot>, ref<HeliogrimApi::Type_>) {}

HELIOGRIM_BENCH_NOOP_STRUCTURE(NumStub)
HELIOGRIM_BENCH_NOOP_STRUCTURE(MixStub)

#undef HELIOGRIM_BENCH_NOOP_STRUCTURE

template class hg::InheritArchetype<HeliogrimApi::BenchNum, hg::Actor>;
template class hg::InheritArchetype<HeliogrimApi::BenchMix, hg::Actor>;

namespace Performance {
	using namespace hg;
	using namespace HeliogrimApi;

	namespace {
		constexpr int runs = 15;

		/* Constructed slot memory of `count_` archetype actors. */
		class Slots {
		public:
			Slots(ref<const ActorClass> class_, const u32 count_) :
				_class(class_),
				_count(count_),
				_memory(
					static_cast<ptr<std::byte>>(
						::operator new(static_cast<size_t>(count_) * class_.desc().stride, std::align_val_t { class_.desc().align })
					)
				) {
				std::memset(_memory, 0, static_cast<size_t>(count_) * class_.desc().stride);
				static Registry instance {};
				_class.ops().construct(_memory, _count, stride(), ::hg::engine::acs::ArchetypeInit { instance });
			}

			Slots(ref<const Slots>) = delete;

			Slots& operator=(ref<const Slots>) = delete;

			~Slots() {
				_class.ops().destroy(_memory, _count, stride());
				::operator delete(_memory, std::align_val_t { _class.desc().align });
			}

		public:
			[[nodiscard]] u32 stride() const noexcept {
				return _class.desc().stride;
			}

			[[nodiscard]] ptr<std::byte> slot(const u32 index_) const noexcept {
				return _memory + static_cast<size_t>(index_) * stride();
			}

		private:
			ref<const ActorClass> _class;
			u32 _count;
			ptr<std::byte> _memory;
		};

		template <typename Fn_>
		[[nodiscard]] double seconds(Fn_&& fn_) {
			const auto start = std::chrono::steady_clock::now();
			fn_();
			return std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
		}

		[[nodiscard]] double medianOf(std::vector<double> values_) {
			std::ranges::sort(values_);
			return values_[values_.size() / 2];
		}

		/* Pins the measuring thread to one core (HG_BENCH_CORE, default 2) and raises its priority. */
		void pinThread() {
			#ifdef _WIN32
			ptr<const char> env = std::getenv("HG_BENCH_CORE");
			const auto core = env != nullptr ? std::strtoull(env, nullptr, 10) : 2uLL;
			SetThreadAffinityMask(GetCurrentThread(), static_cast<DWORD_PTR>(1uLL) << core);
			SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
			#endif
		}

		/**
		 * Layout, memcpy and memmove repetitions alternate within one loop, so drift hits all three alike.
		 * Reports the median absolute GiB/s per method and the median of the per-repetition ratios.
		 */
		template <typename Layout_, typename Copy_, typename Move_>
		void measure(
			ptr<const char> workload_,
			const size_t mib_,
			ptr<const char> op_,
			const size_t bytes_,
			Layout_&& layout_,
			Copy_&& copy_,
			Move_&& move_
		) {
			layout_();
			copy_();
			move_();

			std::vector<double> tl, tc, tm, rc, rm;
			for (int rep = 0; rep < runs; ++rep) {
				const auto l = seconds(layout_);
				const auto c = seconds(copy_);
				const auto m = seconds(move_);
				tl.push_back(l);
				tc.push_back(c);
				tm.push_back(m);
				rc.push_back(c / l);
				rm.push_back(m / l);
			}

			const auto gib = [&](const std::vector<double>& times_) {
				return static_cast<double>(bytes_) / medianOf(times_) / (1024.0 * 1024.0 * 1024.0);
			};
			std::printf(
				"[ BENCH    ] %-8s %4zu MiB %-4s layout %7.2f memcpy %7.2f memmove %7.2f GiB/s  layout/memcpy %.3f  layout/memmove %.3f\n",
				workload_,
				mib_,
				op_,
				gib(tl),
				gib(tc),
				gib(tm),
				medianOf(rc),
				medianOf(rm)
			);
		}

		template <typename Archetype_, typename Stub_>
		void bench(ptr<const char> workload_, void (*fill_)(ref<Stub_>, u32)) {
			#ifndef NDEBUG
			GTEST_SKIP() << "Performance is never measured on Debug.";
			#endif
			pinThread();

			using info = LayoutInfo<typename Archetype_::layout, Archetype_>;
			constexpr size_t headerSize = sizeof(::hg::engine::acs::LayoutHeader);
			constexpr size_t payload = ::hg::engine::acs::payload_size<Stub_>();
			constexpr size_t shift = 8uz;

			ref<const ActorClass> cls = Archetype_::actorClass();
			for (const size_t mib : { 1uz, 16uz, 256uz }) {
				const auto count = static_cast<u32>((mib << 20) / payload);
				const Slots source { cls, count };
				const Slots target { cls, count };
				for (u32 index = 0u; index < count; ++index) {
					fill_(*std::launder(reinterpret_cast<ptr<Stub_>>(source.slot(index) + info::entries[0].offset)), index);
				}

				// Offset of the first streamed member inside a slot, from the live object (Stub_ is not standard layout).
				const size_t memberOffset = reinterpret_cast<const std::byte*>(
					&std::launder(reinterpret_cast<ptr<Stub_>>(source.slot(0u) + info::entries[0].offset))->a
				) - source.slot(0u);

				Vector<std::byte> bytes {};
				cls.ops().dumpLayout(source.slot(0u), count, source.stride(), bytes);
				const size_t streamBytes = bytes.size();
				Vector<std::byte> stream = bytes;
				const auto stride = source.stride();

				// Overlapping move inside one buffer: every slot payload shifts by `shift` bytes.
				Vector<std::byte> scratch = bytes;
				scratch.resize(streamBytes + shift);
				const auto move = [&] {
					auto* base = scratch.data() + headerSize;
					for (u32 index = 0u; index < count; ++index) {
						std::memmove(base + index * payload, base + index * payload + shift, payload);
					}
				};

				measure(
					workload_,
					mib,
					"dump",
					streamBytes,
					[&] {
						bytes.clear();
						cls.ops().dumpLayout(source.slot(0u), count, stride, bytes);
					},
					[&] {
						auto* out = stream.data() + headerSize;
						for (u32 index = 0u; index < count; ++index) {
							std::memcpy(out + index * payload, source.slot(index) + memberOffset, payload);
						}
					},
					move
				);

				// Restore the valid stream for the load side.
				bytes.clear();
				cls.ops().dumpLayout(source.slot(0u), count, stride, bytes);
				stream = bytes;

				measure(
					workload_,
					mib,
					"load",
					streamBytes,
					[&] {
						if (not cls.ops().loadLayout(target.slot(0u), count, stride, stream).has_value()) {
							std::abort();
						}
					},
					[&] {
						const auto* in = stream.data() + headerSize;
						for (u32 index = 0u; index < count; ++index) {
							std::memcpy(target.slot(index) + memberOffset, in + index * payload, payload);
						}
					},
					move
				);
			}
		}
	}

	TEST(LayoutStreamBenchmark, DISABLED_NumericOnly) {
		bench<BenchNum, NumStub>("numeric", [](ref<NumStub> stub_, const u32 index_) {
			stub_.a = index_;
			stub_.b = index_ * 3u;
			stub_.c = index_ ^ 0x5A5Au;
			for (u32 i = 0u; i < 120u; ++i) {
				stub_.d[i] = static_cast<f32>(index_ + i);
			}
		});
	}

	TEST(LayoutStreamBenchmark, DISABLED_Mixed) {
		bench<BenchMix, MixStub>("mixed", [](ref<MixStub> stub_, const u32 index_) {
			stub_.a = index_;
			stub_.b = index_ * 3u;
			stub_.c = index_ ^ 0x5A5Au;
			for (u32 i = 0u; i < 120u; ++i) {
				stub_.d[i] = static_cast<f32>(index_ + i);
			}
			stub_.armed = (index_ & 1u) != 0u;
			stub_.mode = static_cast<BenchMode>(index_ & 3u);
		});
	}
}
