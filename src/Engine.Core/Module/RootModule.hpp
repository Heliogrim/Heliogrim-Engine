#pragma once

#include <concepts>

namespace hg::engine::core {
	class RootModule;

	template <class Type_>
	concept IsRootModule = std::derived_from<Type_, RootModule>;

	/**/

	class macro_novtable RootModule {
	public:
		explicit constexpr RootModule() noexcept = default;

		constexpr ~RootModule() = default;
	};
}
