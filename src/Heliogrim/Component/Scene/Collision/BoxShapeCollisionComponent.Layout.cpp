#include <array>
#include <Heliogrim/Actor/Archetype/ArchetypeLayout.inl.hpp>

#include "BoxShapeCollisionComponent.Layout.hpp"

using namespace hg::engine::serialization;
using namespace hg;

void access::Layout<BoxShapeCollisionComponent>::dump(ref<const BoxShapeCollisionComponent> self_, ref<Vector<std::byte>> out_) {
	// Explicit bits instead of the raw bitfield, so padding bits never leak into the dump.
	std::array<std::byte, sizeof(self_.settings)> settings {};
	settings[0] = static_cast<std::byte>((self_.settings.isStatic ? 1u : 0u) | (self_.settings.isTrigger ? 2u : 0u));
	archetype_detail::put_bytes(out_, settings);
	archetype_detail::put_bytes(out_, self_.extent);
}

void access::Layout<BoxShapeCollisionComponent>::load(std::span<const std::byte> in_, ref<BoxShapeCollisionComponent> target_) {
	std::array<std::byte, sizeof(target_.settings)> settings {};
	std::ignore = archetype_detail::take_bytes(in_, settings);
	target_.settings.isStatic = (std::to_integer<u8>(settings[0]) & 1u) != 0u;
	target_.settings.isTrigger = (std::to_integer<u8>(settings[0]) & 2u) != 0u;
	std::ignore = archetype_detail::take_bytes(in_, target_.extent);
}
