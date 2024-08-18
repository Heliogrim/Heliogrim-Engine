#pragma once

#include <concepts>
#include <type_traits>

namespace hg {
	template <typename AliasType_, typename SourceType_>
	struct as_impl;

	template <typename Type_>
	concept is_aliasable = std::false_type::value;

	template <typename AliasType_, typename SourceType_>
	[[nodiscard]] constexpr AliasType_ as(const SourceType_& val_) {
		return as_impl<AliasType_, SourceType_> {}(val_);
	}

	template <typename AliasType_, typename SourceType_> requires
		(not std::is_pointer_v<AliasType_>) && (not std::is_pointer_v<SourceType_>)
	struct as_impl<const AliasType_&, SourceType_> {
		[[nodiscard]] constexpr const AliasType_& operator()(const SourceType_& val_) const noexcept {
			return val_;
		}
	};
}
