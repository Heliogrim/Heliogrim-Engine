#include "Text.hpp"

#include <format>

#include <Engine.Reflow/Algorithm/Flex.hpp>
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
				0.F, 1.F,
				/**/
				ReflowUnit { ReflowUnitType::eAuto, 0.F },
				ReflowUnit { ReflowUnitType::eAuto, 0.F },
				0.F, 1.F,
				/**/
				Padding { 4.F },
				ReflowPlacement::eMiddleCenter
			}
		},
		{
			TextStyleAttributes {
				TypedAssetGuid<engine::assets::Font> {},
				12.F,
				1.F,
				TextAlign::eTopLeft,
				0uL,
				ReflowWrap::eNone,
				color { 255.F, 255.F, 255.F, 255.F }
			}
		}
	),
	_measureCache() {}

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

bool Text::isBreakPoint(char ch_) const noexcept {
	return std::isspace(ch_) || ch_ == '-' || ch_ == '_';
}

math::fvec2 Text::measure1DimText(ref<const reflow::Font> font_) const {

	const auto& data = getDataAttributes();
	const auto& style = getStyleAttributes();

	const auto& text = data.valueOf<attr::TextData::text>();

	/**/

	if (text.empty()) {
		return math::fvec2 { 0.F, 0.F };
	}

	const auto textHash = hasher::fnv1a<StringView> {}(text);

	if (
		_measureCache.textHash == textHash &&
		_measureCache.fontRef == reinterpret_cast<std::uintptr_t>(std::addressof(font_)) &&
		_measureCache.fontSizeRef == style.valueOf<attr::TextStyle::fontSize>()
	) {
		return math::fvec2 { _measureCache.minExtend, _measureCache.freeExtend };
	}

	/**/

	const auto fss { font_.nextFontSize(static_cast<u32>(style.valueOf<attr::TextStyle::fontSize>())) };
	const auto fontScale { style.valueOf<attr::TextStyle::fontSize>() / static_cast<float>(fss) };

	/**/

	auto sum = f32 { 0.F };
	auto min = f32 { std::numeric_limits<f32>::infinity() };
	auto fwd = f32 { 0.F };
	for (const auto& ch : text) {

		const auto* const glyph = font_.glyph(static_cast<u32>(ch), fss);
		sum += glyph->_advance * fontScale;

		if (isBreakPoint(ch)) {
			min = std::max(min, fwd);
			fwd = 0.F;
			continue;
		}

		fwd += glyph->_advance * fontScale;
	}

	/**/

	_measureCache.textRef = reinterpret_cast<std::uintptr_t>(data.valueOf<attr::TextData::text>().data());
	_measureCache.textHash = textHash;
	_measureCache.fontRef = reinterpret_cast<std::uintptr_t>(std::addressof(font_));
	_measureCache.fontSizeRef = style.valueOf<attr::TextStyle::fontSize>();
	_measureCache.minExtend = std::min(min, fwd);
	_measureCache.freeExtend = sum;

	// Note: We set chunkSum to zero to signal the chunked measure to update
	_measureCache.chunkSum = 0.F;

	/**/

	return math::fvec2 { std::min(min, fwd), sum };
}

f32 Text::measure1CrossDimChunked(ref<const reflow::Font> font_, f32 chunkLimit_) const {

	const auto& data = getDataAttributes();
	const auto& style = getStyleAttributes();

	const auto text = data.valueOf<attr::TextData::text>();

	/**/

	if (text.empty()) {
		return 0.F;
	}

	if (
		// Note: This is used as cache buster from the measure 1-dim text
		_measureCache.chunkSum != 0.F &&
		/* The rest of comparisons is actual cache option validation... */
		_measureCache.textRef == reinterpret_cast<std::uintptr_t>(data.valueOf<attr::TextData::text>().data()) &&
		_measureCache.fontRef == reinterpret_cast<std::uintptr_t>(std::addressof(font_)) &&
		_measureCache.fontSizeRef == style.valueOf<attr::TextStyle::fontSize>() &&
		_measureCache.lineSizeRef == style.valueOf<attr::TextStyle::lineHeight>() &&
		_measureCache.chunkLimitRef == chunkLimit_
	) {
		return _measureCache.chunkSum;
	}

	/**/

	const auto fss { font_.nextFontSize(static_cast<u32>(style.valueOf<attr::TextStyle::fontSize>())) };
	const auto fontScale { style.valueOf<attr::TextStyle::fontSize>() / static_cast<float>(fss) };

	auto lines = text.empty() ? 0uL : 1uL;
	auto lss = f32 { 0.F };
	auto fwd = f32 { 0.F };
	for (const auto& ch : text) {

		const auto* const glyph = font_.glyph(static_cast<u32>(ch), fss);
		const auto advance = glyph->_advance * fontScale;
		lss += advance;

		if (fwd > 0.F && fwd + lss > chunkLimit_) {
			++lines;
			fwd = 0.F;
		}

		if (isBreakPoint(ch)) {
			fwd += lss;
			lss = 0.F;
		}
	}

	/**/

	_measureCache.lineSizeRef = style.valueOf<attr::TextStyle::lineHeight>();
	_measureCache.chunkLimitRef = chunkLimit_;
	// Note: We set chunkSum to zero to signal the chunked measure to update
	_measureCache.chunkSum = static_cast<f32>(lines) *
		style.valueOf<attr::TextStyle::fontSize>() *
		style.valueOf<attr::TextStyle::lineHeight>();

	/**/

	return static_cast<f32>(lines) * style.valueOf<attr::TextStyle::fontSize>() * style.valueOf<attr::TextStyle::lineHeight>();
}

const ptr<const NullChildren> Text::children() const {
	return &_children;
}

void Text::render(const ptr<ReflowCommandBuffer> cmd_) {

	const auto& data = getDataAttributes();
	const auto& layout = getLayoutAttributes();
	const auto& style = getStyleAttributes();

	const auto padding = layout.valueOf<attr::BoxLayout::padding>();
	const auto shift = _layoutState.layoutOffset + math::fvec2 { padding.x, padding.y };
	const auto space = _layoutState.layoutSize - math::fvec2 { padding.x + padding.z, padding.y + padding.w };

	auto font = loadFont(style.valueOf<attr::TextStyle::font>());

	/**/

	const math::fExtent2D scissor { space.x, space.y, shift.x, shift.y };
	cmd_->pushIntersectScissor(scissor);

	/**/

	const u8 align { static_cast<u8>(style.valueOf<attr::TextStyle::textAlign>()) };

	const auto alignVertical =
		/* Vertical Align Top */
		(align & 0b1110'0000u) == 0x0u ?
		0.F :
		/* Vertical Align Middle */
		(align & 0b1111'0000u) == 0x20u ?
		(getLayoutState().layoutSize.y - getLayoutState().computeSize.y) / 2.F :
		/* Vertical Align Bottom */
		(align & 0b1111'0000u) == 0x40u ?
		(getLayoutState().layoutSize.y - getLayoutState().computeSize.y) :
		/* Invalid flag */
		0.F;

	const auto alignShiftFactor =
		/* Align Left */
		(align & 0b0000'1110u) == 0x0u ?
		0.F :
		/* Align Center*/
		(align & 0b0000'1111u) == 0x2u ?
		0.5F :
		/* Align Right*/
		(align & 0b0000'1111u) == 0x4u ?
		1.F :
		/* Invalid flag */
		0.F;

	/**/

	const auto text = StringView { data.valueOf<attr::TextData::text>() };
	const auto fss { font->nextFontSize(static_cast<u32>(style.valueOf<attr::TextStyle::fontSize>())) };
	const auto fontScale = style.valueOf<attr::TextStyle::fontSize>() / static_cast<f32>(fss);
	const auto lineScale = style.valueOf<attr::TextStyle::fontSize>() * style.valueOf<attr::TextStyle::lineHeight>();

	/**/

	const auto renderTextLine = [&](const u16 line_, const u16 start_, const u16 end_, const f32 extend_) {

		const auto alignShift = (space.x - extend_) * alignShiftFactor;
		const auto base = shift + math::fvec2 { alignShift, alignVertical + static_cast<f32>(line_) * lineScale };

		cmd_->drawText(
			base,
			StringView { text.substr(start_, end_ - start_) },
			*font,
			style.valueOf<attr::TextStyle::fontSize>(),
			style.valueOf<attr::TextStyle::color>()
		);

	};

	/**/

	auto lineIdx = u16 { 0uL };
	auto lineStartIdx = u16 { 0uL };
	auto lineFwd = f32 { 0.F };
	auto wordStartIdx = u16 { 0uL };
	auto wordFwd = f32 { 0.F };

	for (size_t idx = 0uLL; idx < text.length(); ++idx) {

		const auto ch = text[idx];
		const auto* const glyph = font->glyph(static_cast<u32>(ch), fss);

		if (glyph == nullptr) {
			continue;
		}

		const auto advance = glyph->_advance * fontScale;
		if (lineFwd > 0.F && lineFwd + wordFwd + advance > space.x) {
			renderTextLine(lineIdx, lineStartIdx, wordStartIdx, lineFwd);

			++lineIdx;
			lineStartIdx = wordStartIdx;
			lineFwd = 0.F;
		}

		/**/

		if (isBreakPoint(ch) && wordStartIdx == idx) {
			lineFwd += advance;
			wordFwd = 0.F;
			wordStartIdx = idx + 1;
			continue;
		}

		if (isBreakPoint(ch)) {
			lineFwd += wordFwd + advance;
			wordFwd = 0.F;
			wordStartIdx = idx + 1;
			continue;
		}

		wordFwd += advance;
	}

	if (lineFwd > 0.F || wordFwd > 0.F) {
		renderTextLine(lineIdx, lineStartIdx, text.length(), lineFwd + wordFwd);
	}

	if (not style.valueOf<attr::TextStyle::textEllipse>()) {
		// TODO: Handle text ellipsing
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
			stored.update<attr::BoxLayout::maxWidth>(layout->valueOf<attr::BoxLayout::maxWidth>());
			stored.update<attr::BoxLayout::widthGrow>(layout->valueOf<attr::BoxLayout::widthGrow>());
			stored.update<attr::BoxLayout::widthShrink>(layout->valueOf<attr::BoxLayout::widthShrink>());
			stored.update<attr::BoxLayout::minHeight>(layout->valueOf<attr::BoxLayout::minHeight>());
			stored.update<attr::BoxLayout::maxHeight>(layout->valueOf<attr::BoxLayout::maxHeight>());
			stored.update<attr::BoxLayout::heightGrow>(layout->valueOf<attr::BoxLayout::heightGrow>());
			stored.update<attr::BoxLayout::heightShrink>(layout->valueOf<attr::BoxLayout::heightShrink>());
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

PrefetchSizing Text::prefetchSizing(ReflowAxis axis_, ref<const ReflowState> state_) const {

	const auto& box = getLayoutAttributes();
	const auto& style = getStyleAttributes();

	const auto fb = algorithm::PrefetchBox {
		.mainAxis = ReflowAxis::eXAxis,
		.min = algorithm::unitAbsMin(box.valueOf<attr::BoxLayout::minWidth>(), box.valueOf<attr::BoxLayout::minHeight>()),
		.max = algorithm::unitAbsMax(box.valueOf<attr::BoxLayout::maxWidth>(), box.valueOf<attr::BoxLayout::maxHeight>()),
		.gapping = { 0.F, 0.F },
		.padding = box.valueOf<attr::BoxLayout::padding>()
	};
	auto base = algorithm::prefetch(axis_, fb, children());

	if (axis_ == ReflowAxis::eXAxis) {

		// TODO: Measure text requirements
		const auto font = loadFont(style.valueOf<attr::TextStyle::font>());
		const auto textSizing = measure1DimText(*font);

		base.minSizing.x += textSizing.min;
		base.sizing.x += textSizing.max;

	} else {

		const auto font = loadFont(style.valueOf<attr::TextStyle::font>());
		const auto textSizing = measure1CrossDimChunked(*font, getLayoutState().computeSize.x - fb.padding.x - fb.padding.z);

		base.minSizing.y += textSizing;
		base.sizing.y += textSizing;

	}

	return {
		math::compClamp(fb.min, base.minSizing, fb.max),
		math::compClamp(fb.min, base.sizing, fb.max)
	};
}

PassPrefetchSizing Text::passPrefetchSizing(ReflowAxis axis_, ref<const ReflowPassState> passState_) const {

	const auto& box = getLayoutAttributes();
	return {
		algorithm::nextUnit(
			box.valueOf<attr::BoxLayout::minWidth>(),
			box.valueOf<attr::BoxLayout::minHeight>(),
			passState_.referenceSize,
			passState_.prefetchMinSize
		),
		math::compMax(
			algorithm::nextUnit(
				box.valueOf<attr::BoxLayout::minWidth>(),
				box.valueOf<attr::BoxLayout::minHeight>(),
				passState_.referenceSize,
				passState_.prefetchSize
			),
			passState_.prefetchSize
		),
		algorithm::nextUnit(
			box.valueOf<attr::BoxLayout::maxWidth>(),
			box.valueOf<attr::BoxLayout::maxHeight>(),
			passState_.referenceSize,
			algorithm::unitAbsMax(
				box.valueOf<attr::BoxLayout::maxWidth>(),
				box.valueOf<attr::BoxLayout::maxHeight>()
			)
		)
	};
}

math::fvec2 Text::computeReferenceSize(ReflowAxis axis_) const {
	return math::vec2_zero;
}

void Text::computeSizing(ReflowAxis axis_, ref<const ReflowPassState> passState_) {
	// Note: We artificially employ the prefetchSizing function to handle wrapping on secondary pass
}

void Text::applyLayout(ref<ReflowState> state_) {}

math::fvec2 Text::getGrowFactor() const noexcept {
	return {
		getLayoutAttributes().valueOf<attr::BoxLayout::widthGrow>(),
		getLayoutAttributes().valueOf<attr::BoxLayout::heightGrow>()
	};
}

math::fvec2 Text::getShrinkFactor() const noexcept {
	return {
		getLayoutAttributes().valueOf<attr::BoxLayout::widthShrink>(),
		getLayoutAttributes().valueOf<attr::BoxLayout::heightShrink>()
	};
}

/**/

#include <Engine.Assets/Assets.hpp>
#include <Engine.Assets.System/IAssetRegistry.hpp>
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
