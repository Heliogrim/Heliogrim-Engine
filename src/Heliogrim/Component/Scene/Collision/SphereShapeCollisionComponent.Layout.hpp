#pragma once

#include <span>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Serialization/Access/__fwd.hpp>

#include "SphereShapeCollisionComponent.hpp"

namespace hg::engine::serialization::access {
	template <>
	class Layout<::hg::SphereShapeCollisionComponent> {
	public:
		constexpr static u64 version = 1uLL;
		constexpr static u64 size = sizeof(::hg::SphereShapeCollisionComponent::Settings) + sizeof(math::Vector1f32);

	public:
		static void dump(ref<const ::hg::SphereShapeCollisionComponent> self_, ref<Vector<std::byte>> out_);

		static void load(std::span<const std::byte> in_, ref<::hg::SphereShapeCollisionComponent> target_);
	};
}
