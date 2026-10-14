#pragma once

#include <std23/function_ref.h>

namespace hg {
	template <typename Fn_, typename... Args_>
	using FnRef = std23::function_ref<Fn_, Args_...>;

	//template <auto V_>
	//using nontype_t = std23::nontype_t<V_>;

	//template <typename Fn_, typename... Args_>
	//struct FnRef : public std23::function_ref<Fn_, Args_...> {
	//	using base_type = std23::function_ref<Fn_, Args_...>;
	//	using typename base_type::function_ref;
	//};

	/**/

	//template <auto V>
	//constexpr inline nontype_t<V> nontype {};

	/**/

	//template <class F> requires std::is_function_v<F>
	//FnRef(F*) -> FnRef<F>;

	//template <auto V>
	//FnRef(nontype_t<V>) -> FnRef<std23::_adapt_signature_t<decltype(V)>>;

	//template <auto V, class T>
	//FnRef(nontype_t<V>, T&&) -> FnRef<std23::_drop_first_arg_to_invoke_t<decltype(V), T>>;
}
