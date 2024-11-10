#pragma once

#include <Engine.Common/__macro.hpp>

namespace hg::engine::storage {
	class macro_novtable IoResourceBase {
	public:
		constexpr IoResourceBase() noexcept = default;

		constexpr virtual ~IoResourceBase() noexcept = default;

	public:
		[[nodiscard]] virtual bool isShared() const noexcept = 0;
	};
}
