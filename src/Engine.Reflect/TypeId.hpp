#pragma once

#include <type_traits>

#include "Template/QueryTypeId.hpp"

namespace hg::refl {
	template <typename Type_>
	struct TypeIdFn;

	/**/

	template <typename Type_> requires (not std::is_void_v<Type_>)
	struct TypeIdFn<Type_> {
		[[nodiscard]] consteval auto operator()() const noexcept {
			constexpr typename query_type_id<Type_>::result query {};
			return query();
		}
	};

	/**/

	template <typename Type_>
	constexpr inline TypeIdFn<Type_> TypeId {};
}
