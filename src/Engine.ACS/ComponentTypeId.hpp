#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Meta/TypeId.hpp>

namespace hg {
	struct component_type_id :
		public type_id {
		constexpr component_type_id() noexcept = default;

		explicit constexpr component_type_id(const type_id& other_) :
			type_id(other_) {}

		explicit constexpr component_type_id(const decltype(type_id::data) value_) :
			type_id(value_) {}

		constexpr component_type_id(cref<component_type_id> other_) noexcept :
			type_id(other_.data) {}
	};
}

namespace std {
	template <>
	struct hash<hg::component_type_id> :
		public std::hash<hg::type_id> {
		[[nodiscard]] std::size_t operator()(hg::cref<hg::component_type_id> typeId_) const noexcept {
			return static_cast<const std::hash<hg::type_id>&>(*this)(typeId_);
		}
	};

	template <>
	struct less<hg::component_type_id> :
		public std::less<hg::type_id> {
		[[nodiscard]] bool operator()(
			hg::cref<hg::component_type_id> left_,
			hg::cref<hg::component_type_id> right_
		) const noexcept {
			auto& self { static_cast<const std::less<hg::type_id>&>(*this) };
			return self(left_, right_);
		}
	};

	template <>
	struct equal_to<hg::component_type_id> :
		public std::equal_to<hg::type_id> {
		[[nodiscard]] bool operator()(
			hg::cref<hg::component_type_id> left_,
			hg::cref<hg::component_type_id> right_
		) const noexcept {
			auto& self { static_cast<const std::equal_to<hg::type_id>&>(*this) };
			return self(left_, right_);
		}
	};
}
