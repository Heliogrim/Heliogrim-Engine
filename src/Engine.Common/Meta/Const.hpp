#pragma once

namespace hg::meta {
	template <typename Type_>
	struct drop_const {
		using type = Type_;
	};

	template <typename Type_>
	struct drop_const<const Type_> {
		using type = Type_;
	};

	template <typename Type_>
	struct drop_const<const Type_&> {
		using type = Type_&;
	};

	template <typename Type_>
	struct drop_const<const Type_&&> {
		using type = Type_&&;
	};

	template <typename Type_>
	using drop_const_t = typename drop_const<Type_>::type;
}
