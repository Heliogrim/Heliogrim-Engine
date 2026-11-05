#pragma once

#include <span>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Serialization/Access/__fwd.hpp>

#include "StaticGeometryComponent.hpp"

namespace hg::engine::serialization::access {
	template <>
	class Layout<::hg::StaticGeometryComponent> {
	public:
		// shortcut: fixed material capacity, extend layout contract to variable size if exceeded
		constexpr static u64 material_capacity = 16uLL;
		constexpr static u64 version = 1uLL;
		constexpr static u64 size = sizeof(AssetGuid) * (1uLL + material_capacity) + sizeof(u64);

	public:
		static void dump(ref<const ::hg::StaticGeometryComponent> self_, ref<Vector<std::byte>> out_);

		static void load(std::span<const std::byte> in_, ref<::hg::StaticGeometryComponent> target_);
	};
}
