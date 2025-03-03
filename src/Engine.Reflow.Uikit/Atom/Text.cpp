#include "Text.hpp"

#include <format>
#include <Engine.Reflow/Layout/Style.hpp>

using namespace hg::engine::reflow::uikit;
using namespace hg::engine::reflow;
using namespace hg;

/**/

[[nodiscard]] static nmpt<Font> loadFont(cref<TypedAssetGuid<::hg::engine::assets::Font>> guid_);

/**/

Text::Text() :
	Text(ReflowClassList { "[Text]"sv }, nullptr) {}

Text::Text(mref<ReflowClassList> classList_, mref<SharedPtr<Widget>> parent_) :
	Widget(::hg::move(classList_), ::hg::move(parent_)),
	Atom(
		{},
		{
			BoxLayoutAttributes {
				ReflowUnit { ReflowUnitType::eAuto, 0.F },
				ReflowUnit { ReflowUnitType::eAuto, 0.F },
				ReflowUnit { ReflowUnitType::eAuto, 0.F },
				ReflowUnit { ReflowUnitType::eAuto, 0.F },
				ReflowUnit { ReflowUnitType::eAuto, 0.F },
				ReflowUnit { ReflowUnitType::eAuto, 0.F },
				Padding { 4.F },
				ReflowPlacement::eMiddleCenter
			}
		},
		{
			TextStyleAttributes {
				TypedAssetGuid<engine::assets::Font> {},
				12.F,
				1.F,
				TextAlign::eLeftTop,
				0uL,
				ReflowWrap::eNone,
				color { 255.F, 255.F, 255.F, 255.F }
			}
		}
	) {}

Text::~Text() = default;

string Text::getTag() const noexcept {
	return std::format(R"(Text <{:#x}>)", reinterpret_cast<u64>(this));
}

string Text::getText() const noexcept {
	return getDataAttributes().valueOf<attr::TextData::text>();
}

void Text::setText(cref<string> text_) {
	getDataAttributes().update<attr::TextData::text>(text_);
	markAsPending();
}

math::vec2 Text::contentSize(cref<reflow::Font> font_, cref<math::vec2> space_) const {

	const auto& data = getDataAttributes();
	const auto& style = getStyleAttributes();

	/**/

	const auto fss { font_.nextFontSize(static_cast<u32>(style.valueOf<attr::TextStyle::fontSize>())) };
	const auto fontScale { style.valueOf<attr::TextStyle::fontSize>() / static_cast<float>(fss) };

	float lineBound = space_.x;

	float fwd { 0.F };
	u32 lines { 0uL };
	math::vec2 size { 0.F };

	for (const auto& letter : data.valueOf<attr::TextData::text>()) {

		const auto* glyph { font_.glyph(static_cast<u32>(letter), fss) };

		if (glyph == nullptr) {
			continue;
		}

		fwd += glyph->_advance * fontScale;

		if (style.valueOf<attr::TextStyle::textWrap>() == ReflowWrap::eWrap && fwd > lineBound) {
			fwd = glyph->_advance * fontScale;
			++lines;
		}

		size.x = MAX(size.x, fwd);
	}

	/**/

	if (style.valueOf<attr::TextStyle::textWrap>() == ReflowWrap::eNoWrap) {
		lines = 1uL;
	} else if (style.valueOf<attr::TextStyle::textEllipse>()) {
		lines = MIN(lines, style.valueOf<attr::TextStyle::textEllipse>());
	}

	size.y = static_cast<float>(lines) * style.valueOf<attr::TextStyle::fontSize>() * style.valueOf<attr::TextStyle::lineHeight>();
	return size;
}

const ptr<const NullChildren> Text::children() const {
	return &_children;
}

void Text::render(const ptr<ReflowCommandBuffer> cmd_) {

	const auto& data = getDataAttributes();
	const auto& style = getStyleAttributes();

	math::vec2 innerOffset = _layoutState.layoutOffset;
	const math::vec2 innerSize = _layoutState.layoutSize;

	auto font = loadFont(style.valueOf<attr::TextStyle::font>());
	const math::vec2 occupied = contentSize(*font, innerSize);

	/**/

	const math::fExtent2D scissor { innerSize.x, innerSize.y, innerOffset.x, innerOffset.y };
	cmd_->pushIntersectScissor(scissor);

	/**/

	const u8 align { static_cast<u8>(style.valueOf<attr::TextStyle::textAlign>()) };
	if ((align & 0b0000'1110u) == 0x0u) {

		/* Align Left */
		// off.x += _computedStyle.margin.x;

	} else if ((align & 0b0000'1111u) == 0x2u) {

		/* Align Center */
		const math::vec2 diff { innerSize.x - occupied.x, innerSize.y - occupied.y };
		innerOffset.x += diff.x * 0.5F;

	} else if ((align & 0b0000'1111u) == 0x4u) {

		/* Align Right */
		const math::vec2 diff { innerSize.x - occupied.x, innerSize.y - occupied.y };
		innerOffset.x += diff.x;
	}

	if ((align & 0b1110'0000u) == 0x0u) {

		/* Vertical Align Top */
		// off.y += _computedStyle.margin.y;

	} else if ((align & 0b1111'0000u) == 0x20u) {

		/* Vertical Align Middle */
		const math::vec2 diff { innerSize.x - occupied.x, innerSize.y - occupied.y };
		innerOffset.y += diff.y * 0.5F;

	} else if ((align & 0b1111'0000u) == 0x40u) {

		/* Vertical Align Bottom */
		const math::vec2 diff { innerSize.x - occupied.x, innerSize.y - occupied.y };
		innerOffset.y += diff.y;
	}

	/**/

	if (not style.valueOf<attr::TextStyle::textEllipse>()) {
		cmd_->drawText(
			innerOffset,
			data.valueOf<attr::TextData::text>(),
			*font,
			style.valueOf<attr::TextStyle::fontSize>(),
			style.valueOf<attr::TextStyle::color>()
		);

	} else {

		const auto fss { font->nextFontSize(static_cast<u32>(style.valueOf<attr::TextStyle::fontSize>())) };
		const auto fontScale { style.valueOf<attr::TextStyle::fontSize>() / static_cast<float>(fss) };
		const auto lineBound { occupied.x };

		float fccw { 0.F };
		u32 wraps { 0uL };
		u32 j { 0uL };

		// Warning: We need to use references or string_views instead of string copies
		const auto text = StringView { data.valueOf<attr::TextData::text>() };
		for (u32 i { 0uL }; i < text.length(); ++i) {

			const auto& letter = text[i];
			const auto* glyph { font->glyph(static_cast<u32>(letter), fss) };

			if (glyph == nullptr) {
				continue;
			}

			fccw += glyph->_advance * fontScale;

			if (style.valueOf<attr::TextStyle::textWrap>() == ReflowWrap::eWrap &&
				fccw > lineBound &&
				wraps < style.valueOf<attr::TextStyle::textEllipse>()
			) {

				cmd_->drawText(
					innerOffset,
					string_view { text.begin() + j, text.begin() + i },
					*font,
					style.valueOf<attr::TextStyle::fontSize>(),
					style.valueOf<attr::TextStyle::color>()
				);

				innerOffset.y += style.valueOf<attr::TextStyle::fontSize>() * style.valueOf<attr::TextStyle::lineHeight>();

				fccw = glyph->_advance * fontScale;
				j = i;
				++wraps;
			}

		}

		if (fccw <= lineBound && wraps < style.valueOf<attr::TextStyle::textEllipse>()) {
			cmd_->drawText(
				innerOffset,
				string_view { text.begin() + j, text.end() },
				*font,
				style.valueOf<attr::TextStyle::fontSize>(),
				style.valueOf<attr::TextStyle::color>()
			);
		}

	}

	/**/

	cmd_->popScissor();
}

void Text::cascadeContextChange(bool invalidate_) {

	if (getEffectiveProvisioner() != None) {

		const auto& theme = getEffectiveProvisioner()->getContext();

		Opt<ref<const BoxLayoutAttributes>> layout = None;
		Opt<ref<const TextStyleAttributes>> style = None;

		for (const auto& className : _staticClassList) {
			if (not layout.has_value() && className.is<StringView>()) {
				layout = theme.getBoxLayout(className.as<StringView>());
			}
			if (not style.has_value() && className.is<StringView>()) {
				style = theme.getTextStyle(className.as<StringView>());
			}
			if (layout.has_value() && style.has_value()) {
				break;
			}
		}

		if (layout != None) {
			auto& stored = getLayoutAttributes();

			stored.update<attr::BoxLayout::minWidth>(layout->valueOf<attr::BoxLayout::minWidth>());
			stored.update<attr::BoxLayout::width>(layout->valueOf<attr::BoxLayout::width>());
			stored.update<attr::BoxLayout::maxWidth>(layout->valueOf<attr::BoxLayout::maxWidth>());
			stored.update<attr::BoxLayout::minHeight>(layout->valueOf<attr::BoxLayout::minHeight>());
			stored.update<attr::BoxLayout::height>(layout->valueOf<attr::BoxLayout::height>());
			stored.update<attr::BoxLayout::maxHeight>(layout->valueOf<attr::BoxLayout::maxHeight>());
			stored.update<attr::BoxLayout::padding>(layout->valueOf<attr::BoxLayout::padding>());
			stored.update<attr::BoxLayout::placement>(layout->valueOf<attr::BoxLayout::placement>());
		}

		if (style != None) {
			auto& stored = getStyleAttributes();

			stored.update<attr::TextStyle::font>(style->valueOf<attr::TextStyle::font>());
			stored.update<attr::TextStyle::fontSize>(style->valueOf<attr::TextStyle::fontSize>());
			stored.update<attr::TextStyle::lineHeight>(style->valueOf<attr::TextStyle::lineHeight>());
			stored.update<attr::TextStyle::textAlign>(style->valueOf<attr::TextStyle::textAlign>());
			stored.update<attr::TextStyle::textEllipse>(style->valueOf<attr::TextStyle::textEllipse>());
			stored.update<attr::TextStyle::textWrap>(style->valueOf<attr::TextStyle::textWrap>());
			stored.update<attr::TextStyle::color>(style->valueOf<attr::TextStyle::color>());
		}

	}

	/**/

	Widget::cascadeContextChange(invalidate_);
}

math::vec2 Text::prefetchDesiredSize(cref<ReflowState> state_, float scale_) const {

	const auto& layout = getLayoutAttributes();
	const auto& style = getStyleAttributes();

	/**/

	math::vec2 size { std::numeric_limits<float>::infinity() };
	if (layout.valueOf<attr::BoxLayout::width>().type == ReflowUnitType::eAbsolute) {
		size.x = layout.valueOf<attr::BoxLayout::width>().value;
	}
	if (layout.valueOf<attr::BoxLayout::height>().type == ReflowUnitType::eAbsolute) {
		size.y = layout.valueOf<attr::BoxLayout::height>().value;
	}

	/**/

	const auto font = loadFont(style.valueOf<attr::TextStyle::font>());
	auto baseSize = contentSize(*font, size);
	baseSize *= scale_;

	/**/

	if (layout.valueOf<attr::BoxLayout::width>().type == ReflowUnitType::eAbsolute) {
		size.x = layout.valueOf<attr::BoxLayout::width>().value;
	} else {
		size.x = baseSize.x;
	}

	if (layout.valueOf<attr::BoxLayout::height>().type == ReflowUnitType::eAbsolute) {
		size.y = layout.valueOf<attr::BoxLayout::height>().value;
	} else {
		size.y = baseSize.y;
	}

	/**/

	//return layout::clampSizeAbs(layout, size);
	return size;
}

math::vec2 Text::computeDesiredSize(cref<ReflowPassState> passState_) const {

	const auto& layout = getLayoutAttributes();
	const auto& style = getStyleAttributes();

	math::vec2 desired { getDesiredSize() };
	if (layout.valueOf<attr::BoxLayout::width>().type == ReflowUnitType::eRelative) {
		desired.x = passState_.referenceSize.x * layout.valueOf<attr::BoxLayout::width>().value;
	}
	if (layout.valueOf<attr::BoxLayout::height>().type == ReflowUnitType::eRelative) {
		desired.y = passState_.referenceSize.y * layout.valueOf<attr::BoxLayout::height>().value;
	}

	/**/

	const auto font = loadFont(style.valueOf<attr::TextStyle::font>());
	auto baseSize = contentSize(*font, desired);
	baseSize *= 1.F;

	/**/

	if (layout.valueOf<attr::BoxLayout::width>().type == ReflowUnitType::eAbsolute) {
		desired.x = layout.valueOf<attr::BoxLayout::width>().value;
	} else if (layout.valueOf<attr::BoxLayout::width>().type == ReflowUnitType::eRelative) {
		desired.x = passState_.referenceSize.x * layout.valueOf<attr::BoxLayout::width>().value;
	} else {
		desired.x = baseSize.x;
	}

	if (layout.valueOf<attr::BoxLayout::height>().type == ReflowUnitType::eAbsolute) {
		desired.y = layout.valueOf<attr::BoxLayout::height>().value;
	} else if (layout.valueOf<attr::BoxLayout::height>().type == ReflowUnitType::eRelative) {
		desired.y = passState_.referenceSize.y * layout.valueOf<attr::BoxLayout::height>().value;
	} else {
		desired.y = baseSize.y;
	}

	/**/

	//return layout::clampSize(layout, passState_.layoutSize, desired);
	return desired;
}

void Text::applyLayout(ref<ReflowState> state_, mref<LayoutContext> ctx_) {
	// TODO: Implement
}

/**/

#include <Engine.Assets.System/IAssetRegistry.hpp>
#include <Engine.Assets/Assets.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.GFX.Loader/Font/FontLoadOptions.hpp>
#include <Engine.GFX.Loader/Font/FontResource.hpp>
#include <Engine.GFX.Loader/Font/Traits.hpp>
#include <Engine.Reflect/Cast.hpp>
#include <Engine.Resource/ResourceManager.hpp>

nmpt<Font> loadFont(cref<TypedAssetGuid<hg::engine::assets::Font>> guid_) {

	const auto engine = ::hg::engine::Engine::getEngine();
	const auto asset = engine->getAssets()->getRegistry()->findAssetByGuid(guid_);
	::hg::assertrt(asset != None);

	auto* fontAsset = Cast<::hg::engine::assets::Font>(asset->get());
	return engine->getResources()->loader().load<engine::assets::Font, engine::gfx::FontResource>(
		std::move(fontAsset),
		engine::gfx::loader::FontLoadOptions {
			.ranges = { BasicLatin, Latin1Supplement },
			.glyphs = {},
			.fontSizes = { 12uL, 16uL, 24uL }
		}
	)->acquire(engine::resource::ResourceUsageFlag::eAll).mut();
}
