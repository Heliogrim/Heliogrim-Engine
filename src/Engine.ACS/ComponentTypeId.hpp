#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Meta/TypeId.hpp>

namespace hg {
	struct ComponentTypeId :
		type_id {
		constexpr ComponentTypeId() noexcept = default;

		explicit constexpr ComponentTypeId(const type_id& other_) :
			type_id(other_) {}

		explicit constexpr ComponentTypeId(const decltype(type_id::data) value_) :
			type_id(value_) {}
	};
}

namespace std {
	template <>
	struct hash<hg::ComponentTypeId> :
		public std::hash<hg::type_id> {
		[[nodiscard]] std::size_t operator()(hg::cref<hg::ComponentTypeId> typeId_) const noexcept {
			return static_cast<const std::hash<hg::type_id>&>(*this)(typeId_);
		}
	};

	template <>
	struct less<hg::ComponentTypeId> :
		public std::less<hg::type_id> {
		[[nodiscard]] bool operator()(
			hg::cref<hg::ComponentTypeId> left_,
			hg::cref<hg::ComponentTypeId> right_
		) const noexcept {
			auto& self { static_cast<const std::less<hg::type_id>&>(*this) };
			return self(left_, right_);
		}
	};

	template <>
	struct equal_to<hg::ComponentTypeId> :
		public std::equal_to<hg::type_id> {
		[[nodiscard]] bool operator()(
			hg::cref<hg::ComponentTypeId> left_,
			hg::cref<hg::ComponentTypeId> right_
		) const noexcept {
			auto& self { static_cast<const std::equal_to<hg::type_id>&>(*this) };
			return self(left_, right_);
		}
	};
}
