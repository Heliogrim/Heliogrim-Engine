#pragma once

#include <map>
#include <Engine.Common/Optional.hpp>
#include <Engine.Common/String.hpp>
#include <Engine.Common/Memory/UniquePointer.hpp>

#include "Theme.hpp"

namespace hg::engine::reflow::theming {
	struct ThemeStateOptions {
		bool focus : 1;
		bool hover : 1;
		bool disabled : 1;
		bool pressed : 1;
		bool dragged : 1;
		u8 pad0 : 3;
	};

	class Theming {
	public:
		using this_type = Theming;

	public:
		Theming() noexcept;

		Theming(ref<const this_type>) = delete;

		Theming(mref<this_type>) = delete;

		~Theming() noexcept;

	private:
		std::map<size_t, UniquePtr<Theme>> _themes;

	public:
		[[nodiscard]] Opt<ref<const Theme>> getBaseTheme() const noexcept;

		[[nodiscard]] Opt<ref<const Theme>> getTheme(StringView key_) const noexcept;

		[[nodiscard]] Opt<ref<const Theme>> getStatedTheme(u8 level_, ThemeStateOptions states_) const noexcept;

		void storeBaseTheme(mref<Theme> theme_);

		void storeTheme(StringView key_, mref<Theme> theme_);

		void storeStatedTheme(u8 level_, ThemeStateOptions states_, mref<Theme> theme_);
	};
}
