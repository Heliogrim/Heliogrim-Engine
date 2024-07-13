#pragma once

namespace hg {
	template <class... Type_>
	struct Overloaded : public Type_... {
		using Type_::operator()...;
	};

	template <class... Type_>
	Overloaded(Type_...) -> Overloaded<Type_...>;
}
