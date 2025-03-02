#pragma once

#include <span>

#include <Engine.Common/Optional.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>

#include "Theme.hpp"
#include "Engine.Reflow.Attributes/Style/Base/ImageStyleAttributes.hpp"

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
