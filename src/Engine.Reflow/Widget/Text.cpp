#include "Text.hpp"

#include <format>
#include "../Layout/Style.hpp"

using namespace hg::engine::reflow;
using namespace hg;

/**/

[[nodiscard]] static nmpt<Font> loadFont(cref<TypedAssetGuid<::hg::engine::assets::Font>> guid_);

/**/

Text::Text() :
	Widget(),
	attr(
		Attributes {
			.width = { this, { ReflowUnitType::eAuto, 0.F } },
			.height = { this, { ReflowUnitType::eAuto, 0.F } },
			.font = { this, {} },
			.fontSize = { this, 16.F },
			.lineHeight = { this, 1.F },
			.textAlign = { this, TextAlign::eLeftTop },
			.textEllipse = { this, 0uL },
			.textWrap = { this, ReflowWrap::eNone },
			.color = { this, engine::color { 255.F, 255.F, 255.F, 255.F } },
			.text = { this, "" }
		}
	) {}

Text::~Text() = default;

string Text::getTag() const noexcept {
	return std::format(R"(Text <{:#x}>)", reinterpret_cast<u64>(this));
}

string Text::getText() const noexcept {
	return attr.text.getValue();
}

void Text::setText(cref<string> text_) {
	attr.text.setValue(text_);
	markAsPending();
}

math::vec2 Text::contentSize(cref<reflow::Font> font_, cref<math::vec2> space_) const {

	const auto fss { font_.nextFontSize(static_cast<u32>(attr.fontSize.getValue())) };
	const auto fontScale { attr.fontSize.getValue() / static_cast<float>(fss) };

	float lineBound = space_.x;

	float fwd { 0.F };
	u32 lines { 0uL };
	math::vec2 size { 0.F };

	for (const auto& letter : attr.text.getValue()) {

		const auto* glyph { font_.glyph(static_cast<u32>(letter), fss) };

		if (glyph == nullptr) {
			continue;
		}

		fwd += glyph->_advance * fontScale;

		if (attr.textWrap.getValue() == ReflowWrap::eWrap && fwd > lineBound) {
			fwd = glyph->_advance * fontScale;
			++lines;
		}

		size.x = MAX(size.x, fwd);
	}

	/**/

	if (attr.textWrap.getValue() == ReflowWrap::eNoWrap) {
		lines = 1uL;
	} else if (attr.textEllipse.getValue()) {
		lines = MIN(lines, attr.textEllipse.getValue());
	}

	size.y = static_cast<float>(lines) * attr.fontSize.getValue() * attr.lineHeight.getValue();
	return size;
}

const ptr<const NullChildren> Text::children() const {
	return &_children;
}

void Text::render(const ptr<ReflowCommandBuffer> cmd_) {

	math::vec2 innerOffset = _layoutState.layoutOffset;
	const math::vec2 innerSize = _layoutState.layoutSize;

	auto font = loadFont(attr.font.getValue());
	const math::vec2 occupied = contentSize(*font, innerSize);

	/**/

	const math::fExtent2D scissor { innerSize.x, innerSize.y, innerOffset.x, innerOffset.y };
	cmd_->pushIntersectScissor(scissor);

	/**/

	const u8 align { static_cast<u8>(attr.textAlign.getValue()) };
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

	if (not attr.textEllipse.getValue()) {
		cmd_->drawText(
			innerOffset,
			attr.text.getValue(),
			*font,
			attr.fontSize.getValue(),
			attr.color.getValue()
		);

	} else {

		const auto fss { font->nextFontSize(static_cast<u32>(attr.fontSize.getValue())) };
		const auto fontScale { attr.fontSize.getValue() / static_cast<float>(fss) };
		const auto lineBound { occupied.x };

		float fccw { 0.F };
		u32 wraps { 0uL };
		u32 j { 0uL };

		// Warning: We need to use references or string_views instead of string copies
		auto text = attr.text.getValue();
		for (u32 i { 0uL }; i < text.length(); ++i) {

			const auto& letter = text[i];
			const auto* glyph { font->glyph(static_cast<u32>(letter), fss) };

			if (glyph == nullptr) {
				continue;
			}

			fccw += glyph->_advance * fontScale;

			if (attr.textWrap.getValue() == ReflowWrap::eWrap &&
				fccw > lineBound &&
				wraps < attr.textEllipse.getValue()
			) {

				cmd_->drawText(
					innerOffset,
					string_view { text.begin() + j, text.begin() + i },
					*font,
					attr.fontSize.getValue(),
					attr.color.getValue()
				);

				innerOffset.y += attr.fontSize.getValue() * attr.lineHeight.getValue();

				fccw = glyph->_advance * fontScale;
				j = i;
				++wraps;
			}

		}

		if (fccw <= lineBound && wraps < attr.textEllipse.getValue()) {
			cmd_->drawText(
				innerOffset,
				string_view { text.begin() + j, text.end() },
				*font,
				attr.fontSize.getValue(),
				attr.color.getValue()
			);
		}

	}

	/**/

	cmd_->popScissor();
}

math::vec2 Text::prefetchDesiredSize(cref<ReflowState> state_, float scale_) const {

	math::vec2 size { std::numeric_limits<float>::infinity() };
	if (attr.width->type == ReflowUnitType::eAbsolute) {
		size.x = attr.width->value;
	}
	if (attr.height->type == ReflowUnitType::eAbsolute) {
		size.y = attr.height->value;
	}

	/**/

	const auto font = loadFont(attr.font.getValue());
	auto baseSize = contentSize(*font, size);
	baseSize *= scale_;

	/**/

	if (attr.width->type == ReflowUnitType::eAbsolute) {
		size.x = attr.width->value;
	} else {
		size.x = baseSize.x;
	}

	if (attr.height->type == ReflowUnitType::eAbsolute) {
		size.y = attr.height->value;
	} else {
		size.y = baseSize.y;
	}

	/**/

	return size;
}

math::vec2 Text::computeDesiredSize(cref<ReflowPassState> passState_) const {

	math::vec2 desired { getDesiredSize() };
	if (attr.width->type == ReflowUnitType::eRelative) {
		desired.x = passState_.referenceSize.x * attr.width->value;
	}
	if (attr.height->type == ReflowUnitType::eRelative) {
		desired.y = passState_.referenceSize.y * attr.height->value;
	}

	/**/

	const auto font = loadFont(attr.font.getValue());
	auto baseSize = contentSize(*font, desired);
	baseSize *= 1.F;

	/**/

	if (attr.width->type == ReflowUnitType::eAbsolute) {
		desired.x = attr.width->value;
	} else if (attr.width->type == ReflowUnitType::eRelative) {
		desired.x = passState_.referenceSize.x * attr.width->value;
	} else {
		desired.x = baseSize.x;
	}

	if (attr.height->type == ReflowUnitType::eAbsolute) {
		desired.y = attr.height->value;
	} else if (attr.height->type == ReflowUnitType::eRelative) {
		desired.y = passState_.referenceSize.y * attr.height->value;
	} else {
		desired.y = baseSize.y;
	}

	/**/

	return desired;
}

void Text::applyLayout(ref<ReflowState> state_, mref<LayoutContext> ctx_) {
	// TODO: Implement
}

bool Text::willChangeLayout(cref<math::vec2> space_) const noexcept {

	if (_state.isProxyPending()) {
		return true;
	}

	/*
	if (layout::hasStyleChanged(_prevStyle, styleStack_)) {
	    return true;
	}

	if (_prevSpace != space_) {
	    return not layout::hasConstSize(attr);
	}
	 */

	return Widget::willChangeLayout(space_);
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
	auto* fontAsset = Cast<::hg::engine::assets::Font>(asset.get());
	return engine->getResources()->loader().load<engine::assets::Font, engine::gfx::FontResource>(
		std::move(fontAsset),
		engine::gfx::loader::FontLoadOptions {
			.ranges = { BasicLatin, Latin1Supplement },
			.glyphs = {},
			.fontSizes = { 12uL, 16uL, 24uL }
		}
	)->acquire(engine::resource::ResourceUsageFlag::eAll).mut();
}
