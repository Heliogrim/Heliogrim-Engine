#pragma once

#include <span>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Serialization/Access/__fwd.hpp>

#include "BoxShapeGeometryComponent.hpp"

namespace hg::engine::serialization::access {
	template <>
	class Layout<::hg::BoxShapeGeometryComponent> {
	public:
		constexpr static u64 version = 1uLL;
		constexpr static u64 size = sizeof(math::Vector3f32) + sizeof(AssetGuid);

	public:
		static void dump(ref<const ::hg::BoxShapeGeometryComponent> self_, ref<Vector<std::byte>> out_);

		static void load(std::span<const std::byte> in_, ref<::hg::BoxShapeGeometryComponent> target_);
	};
}
