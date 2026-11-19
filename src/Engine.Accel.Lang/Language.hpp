#pragma once

#include <Engine.Common/__macro.hpp>

#include "Dialect.hpp"
#include "Lang/TextBlock.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::accel::lang {
	class Language {
	public:
		constexpr Language() noexcept = default;

		Language(cref<Language> other_) :
			dialect(other_.dialect),
			text() {
			for (const auto& val : other_.text) {
				text.emplace_back(val);
			}
		}

		Language(mref<Language> other_) noexcept = default;

		constexpr ~Language() noexcept = default;

	public:
		Dialect dialect;
		InlineAutoArray<TextBlock, 1uLL> text;
	};
}
