#pragma once

#include "Engine.Reflow.Attributes/Style/Base/ImageStyleAttributes.hpp"
#include "Theme.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::reflow::theming {
	struct ThemeContext {
		Opt<ref<const ThemeContext>> parentContext;
		Vector<nmpt<const Theme>> localThemes;

	public:
		[[nodiscard]] std::span<const nmpt<const Theme>> getLocalThemes() const noexcept;

		[[nodiscard]] bool addLocalTheme(nmpt<const Theme> theme_);

		[[nodiscard]] bool dropLocalTheme(nmpt<const Theme> theme_);

	public:
		[[nodiscard]] Opt<ref<const ImageStyleAttributes>> getImageStyle(StringView className_) const;

		[[nodiscard]] Opt<ref<const TextStyleAttributes>> getTextStyle(StringView className_) const;

		[[nodiscard]] Opt<ref<const PaintStyleAttributes>> getPaintStyle(StringView className_) const;

	public:
		[[nodiscard]] Opt<ref<const BoxLayoutAttributes>> getBoxLayout(StringView className_) const;

		//[[nodiscard]] Opt<FlexLayoutAttributes> getFlexLayout(StringView className_) const;
	};
}
