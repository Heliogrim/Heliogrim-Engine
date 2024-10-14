#pragma once

namespace hg::meta {
	template <typename Type_>
	struct drop_ptr {
		using type = Type_;
	};

	template <typename Type_>
	struct drop_ptr<Type_*> {
		using type = Type_;
	};

	template <typename Type_>
	struct drop_ptr<Type_* const> {
		using type = Type_;
	};

	template <typename Type_>
	using drop_ptr_t = typename drop_ptr<Type_>::type;
}
