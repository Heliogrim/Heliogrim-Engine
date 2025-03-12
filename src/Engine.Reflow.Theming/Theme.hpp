#pragma once

#include <map>
#include <Engine.Common/Optional.hpp>
#include <Engine.Common/String.hpp>
#include <Engine.Reflow.Attributes/Layout/Base/BoxLayoutAttributes.hpp>
#include <Engine.Reflow.Attributes/Layout/Base/FlexLayoutAttributes.hpp>
#include <Engine.Reflow.Attributes/Style/Base/ImageStyleAttributes.hpp>
#include <Engine.Reflow.Attributes/Style/Base/PaintStyle.hpp>
#include <Engine.Reflow.Attributes/Style/Base/TextStyleAttributes.hpp>

namespace hg::engine::reflow::theming {
	class Theme final {
	public:
		using this_type = Theme;

	public:
		explicit Theme() = default;

		//private:
	public:
		std::map<StringView, ImageStyleAttributes> _imageStyles;
		std::map<StringView, PaintStyleAttributes> _paintStyles;
		std::map<StringView, TextStyleAttributes> _textStyles;

		std::map<StringView, BoxLayoutAttributes> _boxStyles;
		std::map<StringView, FlexLayoutAttributes> _flexStyles;

	public:
		[[nodiscard]] Opt<ref<const ImageStyleAttributes>> getImageStyle(StringView className_) const;

		void addImageStyle(StringView className_, mref<ImageStyleAttributes> attributes_);

		[[nodiscard]] Opt<ref<const PaintStyleAttributes>> getPaintStyle(StringView className_) const;

		void addPaintStyle(StringView className_, mref<PaintStyleAttributes> attributes_);

		[[nodiscard]] Opt<ref<const TextStyleAttributes>> getTextStyle(StringView className_) const;

		void addTextStyle(StringView className_, mref<TextStyleAttributes> attributes_);

	public:
		[[nodiscard]] Opt<ref<const BoxLayoutAttributes>> getBoxLayout(StringView className_) const;

		void addBoxLayout(StringView className_, mref<BoxLayoutAttributes> attributes_);

		[[nodiscard]] Opt<ref<const FlexLayoutAttributes>> getFlexLayout(StringView className_) const;

		void addFlexLayout(StringView className_, mref<FlexLayoutAttributes> attributes_);
	};
}
