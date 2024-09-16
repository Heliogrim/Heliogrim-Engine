#pragma once

namespace hg {
	template <typename... Types_>
	struct last_type_of;

	template <typename Ty_, typename NextType_, typename... Types_>
	struct last_type_of<Ty_, NextType_, Types_...> {
		using type = typename last_type_of<NextType_, Types_...>::type;
	};

	template <typename Ty_>
	struct last_type_of<Ty_> {
		using type = Ty_;
	};

	template <typename... Types_>
	using last_type_of_t = typename last_type_of<Types_...>::type;
}
