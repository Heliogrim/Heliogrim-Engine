#pragma once

namespace hg::meta {
	template <typename Type_>
	struct drop_ref {
		using type = Type_;
	};

	template <typename Type_>
	struct drop_ref<Type_&> {
		using type = Type_;
	};

	template <typename Type_>
	struct drop_ref<Type_&&> {
		using type = Type_;
	};

	template <typename Type_>
	using remove_reference_t = typename drop_ref<Type_>::type;

	template <typename Type_>
	using drop_ref_t = typename drop_ref<Type_>::type;
}
