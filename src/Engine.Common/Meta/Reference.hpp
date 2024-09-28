#pragma once

namespace hg::meta {
	template <typename Type_>
	struct remove_ref {
		using type = Type_;
	};

	template <typename Type_>
	struct remove_ref<Type_&> {
		using type = Type_;
	};

	template <typename Type_>
	struct remove_ref<Type_&&> {
		using type = Type_;
	};

	template <typename Type_>
	using remove_reference_t = typename remove_ref<Type_>::type;
}
