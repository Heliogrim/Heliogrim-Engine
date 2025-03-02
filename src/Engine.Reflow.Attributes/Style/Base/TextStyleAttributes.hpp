#pragma once

#include <Engine.Assets.Type/Texture/Font.hpp>
#include <Engine.GFX/Color/Color.hpp>
#include <Engine.Reflow/ReflowPlacement.hpp>
#include <Engine.Reflow/ReflowWrap.hpp>
#include <Engine.Reflow/TextAlign.hpp>
#include <Heliogrim/TypedAssetGuid.hpp>

#include "../StyleAttribute.hpp"
#include "../StyleAttributes.hpp"

namespace hg::engine::reflow {
	namespace attr {
		struct TextStyle {
			constexpr static inline auto font = "font"sv;
			constexpr static inline auto fontSize = "fontSize"sv;
			constexpr static inline auto lineHeight = "lineHeight"sv;

			constexpr static inline auto textAlign = "textAlign"sv;
			constexpr static inline auto textEllipse = "textEllipse"sv;
			constexpr static inline auto textWrap = "textWrap"sv;

			constexpr static inline auto color = "color"sv;
		};
	}

	struct TextStyleAttributes final :
		public StyleAttributes<
			NamedAttribute<attr::TextStyle::font, StyleAttribute<TypedAssetGuid<engine::assets::Font>>>,
			NamedAttribute<attr::TextStyle::fontSize, StyleAttribute<float>>,
			NamedAttribute<attr::TextStyle::lineHeight, StyleAttribute<float>>,
			NamedAttribute<attr::TextStyle::textAlign, StyleAttribute<TextAlign>>,
			NamedAttribute<attr::TextStyle::textEllipse, StyleAttribute<u32>>,
			NamedAttribute<attr::TextStyle::textWrap, StyleAttribute<ReflowWrap>>,
			NamedAttribute<attr::TextStyle::color, StyleAttribute<color>>
		> {
		using StyleAttributes::StyleAttributes;
	};
}
