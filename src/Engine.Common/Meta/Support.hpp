#pragma once

#include "Const.hpp"
#include "Pointer.hpp"
#include "Reference.hpp"

namespace hg::meta {
	template <typename Type_>
	struct peeled {
		using type = drop_const_t<drop_ptr_t<drop_ref_t<Type_>>>;
	};

	template <typename Type_>
	using peeled_t = typename peeled<Type_>::type;
}
