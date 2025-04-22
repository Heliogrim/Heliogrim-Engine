#pragma once

#include <Engine.Common/Meta/TypeId.hpp>

namespace hg {
	struct AssetTypeId :
		type_id {
		constexpr AssetTypeId() noexcept = default;

		explicit constexpr AssetTypeId(const type_id& other_):
			type_id(other_) {}

		explicit constexpr AssetTypeId(const decltype(type_id::data) value_) :
			type_id(value_) {}
	};
}

namespace std {
	template <>
	struct hash<hg::AssetTypeId> :
		public std::hash<hg::type_id> {
		[[nodiscard]] std::size_t operator()(hg::cref<hg::AssetTypeId> typeId_) const noexcept {
			return static_cast<const std::hash<hg::type_id>&>(*this)(typeId_);
		}
	};

	template <>
	struct less<hg::AssetTypeId> :
		public std::less<hg::type_id> {
		[[nodiscard]] bool operator()(
			hg::cref<hg::AssetTypeId> left_,
			hg::cref<hg::AssetTypeId> right_
		) const noexcept {
			auto& self { static_cast<const std::less<hg::type_id>&>(*this) };
			return self(left_, right_);
		}
	};

	template <>
	struct equal_to<hg::AssetTypeId> :
		public std::equal_to<hg::type_id> {
		[[nodiscard]] bool operator()(
			hg::cref<hg::AssetTypeId> left_,
			hg::cref<hg::AssetTypeId> right_
		) const noexcept {
			auto& self { static_cast<const std::equal_to<hg::type_id>&>(*this) };
			return self(left_, right_);
		}
	};
}
