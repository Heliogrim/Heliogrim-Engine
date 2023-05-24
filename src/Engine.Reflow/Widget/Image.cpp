#include "Image.hpp"

#include <format>
#include "../Layout/Style.hpp"

using namespace hg::engine::reflow;
using namespace hg;

Image::Image() :
    Widget(),
    attr(
        Attributes {
            .minWidth = { this, { ReflowUnitType::eAuto, 0.F } },
            .width = { this, { ReflowUnitType::eAuto, 0.F } },
            .maxWidth = { this, { ReflowUnitType::eAuto, 0.F } },
            .minHeight = { this, { ReflowUnitType::eAuto, 0.F } },
            .height = { this, { ReflowUnitType::eAuto, 0.F } },
            .maxHeight = { this, { ReflowUnitType::eAuto, 0.F } },
            .tint = { this, engine::color { 255.F, 255.F, 255.F, 255.F } },
            .image = { this, nullptr }
        }
    ),
    _uvs(
        {
            math::vec2 { 0.F, 0.F },
            math::vec2 { 1.F, 0.F },
            math::vec2 { 1.F, 1.F },
            math::vec2 { 0.F, 1.F }
        }
    ),
    _imageResource(nullptr) {}

Image::~Image() {
    // Warning: Workaround which might cause an crash or error
    if (_imageResource) {
        _imageResource = nullptr;
        attr.image.setValue(nullptr);
    }
}

string Image::getTag() const noexcept {
    return _STD format(R"(Image <{:#x}>)", reinterpret_cast<u64>(this));
}

void Image::setImage(cref<sptr<gfx::ProxyTexture<non_owning_rptr>>> image_, ptr<void> resource_) {
    attr.image.setValue(image_);
    _imageResource = resource_;
}

const ptr<const NullChildren> Image::children() const {
    return &_children;
}

void Image::render(const ptr<ReflowCommandBuffer> cmd_) {

    const auto innerOffset = _layoutState.layoutOffset;
    const auto innerSize = _layoutState.layoutSize;

    gfx::ProxyTexture<non_owning_rptr> proxy { *attr.image->get() };

    cmd_->drawImage(
        math::vec2 { innerOffset.x, innerOffset.y },
        _uvs[0],
        math::vec2 { innerOffset.x + innerSize.x, innerOffset.y },
        _uvs[1],
        math::vec2 { innerOffset.x + innerSize.x, innerOffset.y + innerSize.y },
        _uvs[2],
        math::vec2 { innerOffset.x, innerOffset.y + innerSize.y },
        _uvs[3],
        _STD move(proxy),
        attr.tint.getValue()
    );
}

math::vec2 Image::prefetchDesiredSize(cref<ReflowState> state_, float scale_) const {

    math::vec2 size = math::vec2 { 0.F };

    /**/

    if (attr.width->type == ReflowUnitType::eAbsolute) {
        size.x = attr.width->value;
    }
    if (attr.height->type == ReflowUnitType::eAbsolute) {
        size.y = attr.height->value;
    }

    /**/

    return layout::clampSizeAbs(attr, size);
}

math::vec2 Image::computeDesiredSize(cref<ReflowPassState> passState_) const {

    math::vec2 desired { getDesiredSize() };
    if (attr.width->type == ReflowUnitType::eRelative) {
        desired.x = passState_.referenceSize.x * attr.width->value;
    }
    if (attr.height->type == ReflowUnitType::eRelative) {
        desired.y = passState_.referenceSize.y * attr.height->value;
    }

    return layout::clampSize(
        attr,
        passState_.layoutSize,
        desired
    );
}

void Image::applyLayout(ref<ReflowState> state_, mref<LayoutContext> ctx_) {
    // TODO: Implement
}
