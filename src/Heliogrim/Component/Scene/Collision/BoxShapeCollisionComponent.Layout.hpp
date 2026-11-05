#pragma once

#include <span>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Serialization/Access/__fwd.hpp>

#include "BoxShapeCollisionComponent.hpp"

namespace hg::engine::serialization::access {
	template <>
	class Layout<::hg::BoxShapeCollisionComponent> {
	public:
		constexpr static u64 version = 1uLL;
		constexpr static u64 size = sizeof(::hg::BoxShapeCollisionComponent::Settings) + sizeof(math::Vector3f32);

	public:
		static void dump(ref<const ::hg::BoxShapeCollisionComponent> self_, ref<Vector<std::byte>> out_);

		static void load(std::span<const std::byte> in_, ref<::hg::BoxShapeCollisionComponent> target_);
	};
}
