#include "Theme.hpp"

using namespace hg::engine::reflow::theming;
using namespace hg::engine::reflow;
using namespace hg;

Opt<ref<const ImageStyleAttributes>> Theme::getImageStyle(StringView className_) const {
	const auto maybeIt = _imageStyles.find(className_);
	if (maybeIt != _imageStyles.end()) {
		return Some(maybeIt->second);
	}
	return None;
}

void Theme::addImageStyle(StringView className_, mref<ImageStyleAttributes> attributes_) {
	_imageStyles.insert_or_assign(className_, ::hg::move(attributes_));
}

Opt<ref<const PaintStyleAttributes>> Theme::getPaintStyle(StringView className_) const {
	const auto maybeIt = _paintStyles.find(className_);
	if (maybeIt != _paintStyles.end()) {
		return Some(maybeIt->second);
	}
	return None;
}

void Theme::addPaintStyle(StringView className_, mref<PaintStyleAttributes> attributes_) {
	_paintStyles.insert_or_assign(className_, ::hg::move(attributes_));
}

Opt<ref<const TextStyleAttributes>> Theme::getTextStyle(StringView className_) const {
	const auto maybeIt = _textStyles.find(className_);
	if (maybeIt != _textStyles.end()) {
		return Some(maybeIt->second);
	}
	return None;
}

void Theme::addTextStyle(StringView className_, mref<TextStyleAttributes> attributes_) {
	_textStyles.insert_or_assign(className_, ::hg::move(attributes_));
}

Opt<ref<const BoxLayoutAttributes>> Theme::getBoxLayout(StringView className_) const {
	const auto maybeIt = _boxStyles.find(className_);
	if (maybeIt != _boxStyles.end()) {
		return Some(maybeIt->second);
	}
	return None;
}

void Theme::addBoxLayout(StringView className_, mref<BoxLayoutAttributes> attributes_) {
	_boxStyles.insert_or_assign(className_, ::hg::move(attributes_));
}

Opt<ref<const FlexLayoutAttributes>> Theme::getFlexLayout(StringView className_) const {
	const auto maybeIt = _flexStyles.find(className_);
	if (maybeIt != _flexStyles.end()) {
		return Some(maybeIt->second);
	}
	return None;
}

void Theme::addFlexLayout(StringView className_, mref<FlexLayoutAttributes> attributes_) {
	_flexStyles.insert_or_assign(className_, ::hg::move(attributes_));
}
