#include "Image.hpp"

#include <format>
#include <Engine.Reflow/Algorithm/Flex.hpp>
#include <Engine.Reflow/Layout/Style.hpp>

using namespace hg::engine::reflow::uikit;
using namespace hg::engine::reflow;
using namespace hg;

Image::Image() :
	Image(ReflowClassList { "[Image]"sv }, nullptr) {}

Image::Image(mref<ReflowClassList> classList_, mref<SharedPtr<Widget>> parent_) :
	Widget(::hg::move(classList_), ::hg::move(parent_)),
	Atom({}, {}, { .styleAttributes = ImageStyleAttributes { ReflowAlphaMode::eNone, engine::color { 255.F, 255.F, 255.F, 255.F } } }),
	_uvs(
		{
			math::vec2 { 0.F, 0.F },
			math::vec2 { 1.F, 0.F },
			math::vec2 { 1.F, 1.F },
			math::vec2 { 0.F, 1.F }
		}
	) {}

Image::~Image() {
	// Warning: Workaround which might cause an crash or error
	if (getDataAttributes().valueOf<attr::ImageData::resource>() != nullptr) {
		getDataAttributes().update<attr::ImageData::resource>(nullptr);
		getDataAttributes().update<attr::ImageData::image>(nullptr);
	}
}

string Image::getTag() const noexcept {
	return std::format(R"(Image <{:#x}>)", reinterpret_cast<u64>(this));
}

const ptr<const NullChildren> Image::children() const {
	return &_children;
}

void Image::render(const ptr<ReflowCommandBuffer> cmd_) {

	const auto innerOffset = _layoutState.layoutOffset;
	const auto innerSize = _layoutState.layoutSize;

	cmd_->drawImage(
		math::vec2 { innerOffset.x, innerOffset.y },
		_uvs[0],
		math::vec2 { innerOffset.x + innerSize.x, innerOffset.y },
		_uvs[1],
		math::vec2 { innerOffset.x + innerSize.x, innerOffset.y + innerSize.y },
		_uvs[2],
		math::vec2 { innerOffset.x, innerOffset.y + innerSize.y },
		_uvs[3],
		getDataAttributes().valueOf<attr::ImageData::image>(),
		{
			getStyleAttributes().valueOf<attr::ImageStyle::alpha>(),
			getStyleAttributes().valueOf<attr::ImageStyle::tint>()
		}
	);
}

void Image::cascadeContextChange(bool invalidate_) {

	if (getEffectiveProvisioner() != None) {

		const auto& theme = getEffectiveProvisioner()->getContext();

		Opt<ref<const BoxLayoutAttributes>> layout = None;
		Opt<ref<const ImageStyleAttributes>> style = None;

		for (const auto& className : _staticClassList) {
			if (not layout.has_value() && className.is<StringView>()) {
				layout = theme.getBoxLayout(className.as<StringView>());
			}
			if (not style.has_value() && className.is<StringView>()) {
				style = theme.getImageStyle(className.as<StringView>());
			}
			if (layout.has_value() && style.has_value()) {
				break;
			}
		}

		/**/

		if (layout != None) {
			// TODO: getLayoutAttributes().update<attr::BoxLayout::height>(layout->valueOf<attr::BoxLayout::height>());
			// TODO: ...
		}

		if (style != None) {
			getStyleAttributes().update<attr::ImageStyle::alpha>(style->valueOf<attr::ImageStyle::alpha>());
			getStyleAttributes().update<attr::ImageStyle::tint>(style->valueOf<attr::ImageStyle::tint>());
		}

	}

	/**/
	Widget::cascadeContextChange(invalidate_);
}

PrefetchSizing Image::prefetchSizing(ReflowAxis axis_, ref<const ReflowState> state_) const {
	const auto& box = getLayoutAttributes();
	return algorithm::prefetch(
		axis_,
		{
			.mainAxis = ReflowAxis::eXAxis,
			.min = algorithm::unitAbsMin(box.valueOf<attr::BoxLayout::minWidth>(), box.valueOf<attr::BoxLayout::minHeight>()),
			.max = algorithm::unitAbsMax(box.valueOf<attr::BoxLayout::maxWidth>(), box.valueOf<attr::BoxLayout::maxHeight>()),
			.gapping = { 0.F, 0.F },
			.padding = box.valueOf<attr::BoxLayout::padding>()
		},
		children()
	);
}

PassPrefetchSizing Image::passPrefetchSizing(ReflowAxis axis_, ref<const ReflowPassState> passState_) const {
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

void Image::computeSizing(ReflowAxis axis_, ref<const ReflowPassState> passState_) {}

void Image::applyLayout(ref<ReflowState> state_) {}

math::fvec2 Image::getShrinkFactor() const noexcept {
	return {
		getLayoutAttributes().valueOf<attr::BoxLayout::widthShrink>(),
		getLayoutAttributes().valueOf<attr::BoxLayout::heightShrink>()
	};
}

math::fvec2 Image::getGrowFactor() const noexcept {
	return {
		getLayoutAttributes().valueOf<attr::BoxLayout::widthGrow>(),
		getLayoutAttributes().valueOf<attr::BoxLayout::heightGrow>()
	};
}
