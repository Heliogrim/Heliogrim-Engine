#pragma once

#include <Engine.Common/Meta/TypeId.hpp>

#include "../Compile/TypeId.hpp"

namespace hg::refl {
	template <typename Type_>
	concept TypeQueryable = std::true_type::value;

	template <typename Type_>
	concept IsUdTypeQuery = HasStaticType<Type_>;

	template <typename Type_>
	concept IsCtTypeQuery = not HasStaticType<Type_>;

	/**/

	template <typename, bool>
	struct query_type_id_impl;

	template <typename Type_>
	struct query_type_id_impl<Type_, false> {
		using result = decltype([]() constexpr {
			return ::hg::refl::ctid<Type_>();
		});
	};

	template <typename Type_>
	struct query_type_id_impl<Type_, true> {
		using result = decltype([]() constexpr {
			return Type_::typeId;
		});
	};

	template <TypeQueryable Type_>
	struct query_type_id {
		using result = typename query_type_id_impl<Type_, IsUdTypeQuery<Type_>>::result;
	};
}
