/**
 * Feeds arbitrary bytes to the archetype layout loader. A rejected load must leave the target slots untouched.
 * Probe archetype mirrors `TestLinker` of Heliogrim.Test (plain fields, a mask field and both ref kinds).
 */
#include <Engine.ACS/Archetype/ArchetypeDefinition.inl.hpp>
#include <Engine.ACS/Archetype/ArchetypeLayout.inl.hpp>

import Engine.ACS;
import Engine.Common;
import Engine.Serialization;
import Engine.Graphics.Components;

using namespace hg;
using namespace hg::engine::acs;
using namespace hg::engine::serialization;

namespace {
	struct FuzzLink : HierarchyComponent {
		using HierarchyComponent::HierarchyComponent;
		inline constexpr static ComponentTypeId typeId { "FuzzLink"_typeId };

		ActorRef<Actor> target {};
		ComponentRef<HierarchyComponent> part {};
		u32 tag = 0u;
		bool armed = false;
	};

	class FuzzActor :
		public InheritArchetype<FuzzActor, Actor> {
	public:
		using this_type = FuzzActor;
		inline constexpr static type_id typeId { "FuzzActor"_typeId };

		using layout = ActorLayout<Slot<"link", FuzzLink, Children<Slot<"geometry", StaticGeometryComponent>>>>;

	public:
		explicit FuzzActor(ref<const ActorInitializer> initializer_) :
			InheritArchetype(initializer_) {}
	};
}

template <>
class hg::engine::serialization::access::Layout<FuzzLink> {
public:
	inline constexpr static std::array fields {
		HG_LAYOUT_RAW(FuzzLink, tag, ::hg::u32),
		HG_LAYOUT_MASK(FuzzLink, armed, 0x01u)
	};
	inline constexpr static auto refs = std::tuple { &FuzzLink::target, &FuzzLink::part };
};

template <>
void access::Structure<FuzzLink>::serialize(ref<const FuzzLink> self_, mref<StructScopedSlot> record_) {
	record_.insertSlot<u32>("tag") << self_.tag;
}

template <>
void access::Structure<FuzzLink>::hydrate(ref<const StructScopedSlot> record_, ref<FuzzLink> target_) {
	record_.getSlot<u32>("tag") >> target_.tag;
}

template class hg::InheritArchetype<FuzzActor, hg::Actor>;

namespace {
	constexpr u32 count = 2u;

	/* Constructed slot memory of `count` archetype actors. */
	class Slots {
	public:
		explicit Slots(ref<const ActorClass> class_) :
			_class(class_),
			_memory(
				static_cast<ptr<std::byte>>(
					::operator new(static_cast<size_t>(count) * class_.desc().stride, std::align_val_t { class_.desc().align })
				)
			) {
			static Registry registry {};
			_class.ops().construct(_memory, count, stride(), ArchetypeInit { registry });
		}

		Slots(ref<const Slots>) = delete;

		Slots& operator=(ref<const Slots>) = delete;

		~Slots() {
			_class.ops().destroy(_memory, count, stride());
			::operator delete(_memory, std::align_val_t { _class.desc().align });
		}

		[[nodiscard]] u32 stride() const noexcept { return _class.desc().stride; }

		[[nodiscard]] ptr<std::byte> data() const noexcept { return _memory; }

		[[nodiscard]] size_t size() const noexcept { return static_cast<size_t>(count) * stride(); }

	private:
		ref<const ActorClass> _class;
		ptr<std::byte> _memory;
	};
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* const data_, const size_t size_) {
	ref<const ActorClass> cls = FuzzActor::actorClass();
	const Slots target { cls };

	/* Odd first byte: a valid dump patched by (offset, value) triples from the rest; else raw input. */
	Vector<std::byte> bytes {};
	if (size_ > 0uLL && (data_[0] & 1u) != 0u) {
		const Slots source { cls };
		cls.ops().dumpLayout(source.data(), count, source.stride(), bytes);
		for (size_t at = 1uLL; at + 2uLL < size_; at += 3uLL) {
			bytes[((static_cast<size_t>(data_[at]) << 8uLL) | data_[at + 1uLL]) % bytes.size()] = static_cast<std::byte>(data_[at + 2uLL]);
		}
	} else {
		bytes.assign(reinterpret_cast<const std::byte*>(data_), reinterpret_cast<const std::byte*>(data_) + size_);
	}

	Vector<std::byte> before(target.data(), target.data() + target.size());
	const auto result = cls.ops().loadLayout(target.data(), count, target.stride(), std::span<const std::byte> { bytes });
	if (not result.has_value() && std::memcmp(before.data(), target.data(), before.size()) != 0) {
		std::abort();
	}
	if (result.has_value()) {
		// An accepted input holds no bits outside the masks, so the re-dump must reproduce it. Asset guids are left to
		// the resolve pass, so they are zeroed on both sides.
		Vector<std::byte> again {};
		cls.ops().dumpLayout(target.data(), count, target.stride(), again);
		constexpr u64 linkSize = ::hg::engine::acs::payload_size<FuzzLink>();
		constexpr u64 geometrySize = ::hg::engine::acs::payload_size<StaticGeometryComponent>();
		constexpr u64 guidSize = access::Layout<StaticGeometryComponent>::assets * sizeof(AssetGuid);
		if (again.size() != bytes.size()) {
			std::abort();
		}
		for (u32 slot = 0u; slot < count; ++slot) {
			const u64 guids = sizeof(::hg::engine::acs::LayoutHeader) + (slot + 1uLL) * (linkSize + geometrySize) - guidSize;
			std::memset(bytes.data() + guids, 0, guidSize);
			std::memset(again.data() + guids, 0, guidSize);
		}
		// Fixups are skipped: an actor ref ignores its stored component offset, so it need not round-trip.
		const u64 payloadEnd = sizeof(::hg::engine::acs::LayoutHeader) + count * (linkSize + geometrySize);
		if (std::memcmp(bytes.data(), again.data(), payloadEnd) != 0) {
			std::abort();
		}
	}
	return 0;
}

/* Valid layout dump of the probe archetype; seed for the mutation driver. */
extern "C" size_t HgFuzzSeed(uint8_t* const out_, const size_t cap_) {
	ref<const ActorClass> cls = FuzzActor::actorClass();
	const Slots source { cls };
	Vector<std::byte> bytes {};
	cls.ops().dumpLayout(source.data(), count, source.stride(), bytes);
	const size_t n = bytes.size() < cap_ ? bytes.size() : cap_;
	std::memcpy(out_, bytes.data(), n);
	return n;
}
