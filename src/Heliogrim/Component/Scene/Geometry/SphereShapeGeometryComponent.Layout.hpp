#pragma once

#include <span>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Serialization/Access/__fwd.hpp>

#include "SphereShapeGeometryComponent.hpp"

namespace hg::engine::serialization::access {
	template <>
	class Layout<::hg::SphereShapeGeometryComponent> {
	public:
		constexpr static u64 version = 1uLL;
		constexpr static u64 size = sizeof(math::Vector1f32) + sizeof(AssetGuid);

	public:
		static void dump(ref<const ::hg::SphereShapeGeometryComponent> self_, ref<Vector<std::byte>> out_);

		static void load(std::span<const std::byte> in_, ref<::hg::SphereShapeGeometryComponent> target_);
	};
}
