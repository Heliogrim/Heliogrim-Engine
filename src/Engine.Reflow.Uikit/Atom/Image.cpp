#include "Image.hpp"

#include <format>
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

math::vec2 Image::prefetchDesiredSize(cref<ReflowState> state_, float scale_) const {

	const auto& layout = getLayoutAttributes();
	math::vec2 size = math::vec2 { 0.F };

	/**/

	if (layout.valueOf<attr::BoxLayout::width>().type == ReflowUnitType::eAbsolute) {
		size.x = layout.valueOf<attr::BoxLayout::width>().value;
	}
	if (layout.valueOf<attr::BoxLayout::height>().type == ReflowUnitType::eAbsolute) {
		size.y = layout.valueOf<attr::BoxLayout::height>().value;
	}

	/**/

	return layout::clampSizeAbs(layout, size);
}

math::vec2 Image::computeDesiredSize(cref<ReflowPassState> passState_) const {

	const auto& layout = getLayoutAttributes();
	math::vec2 desired { getDesiredSize() };

	if (layout.valueOf<attr::BoxLayout::width>().type == ReflowUnitType::eRelative) {
		desired.x = passState_.referenceSize.x * layout.valueOf<attr::BoxLayout::width>().value;
	}
	if (layout.valueOf<attr::BoxLayout::height>().type == ReflowUnitType::eRelative) {
		desired.y = passState_.referenceSize.y * layout.valueOf<attr::BoxLayout::height>().value;
	}

	return layout::clampSize(layout, passState_.layoutSize, desired);
}

void Image::applyLayout(ref<ReflowState> state_, mref<LayoutContext> ctx_) {
	//
}
