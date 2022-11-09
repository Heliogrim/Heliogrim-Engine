#include "Image.hpp"

#include "../Style/BoundStyleSheet.hpp"

using namespace ember::engine::reflow;
using namespace ember;

Image::Image(mref<sptr<BoundStyleSheet>> style_) :
    Widget(),
    _style(_STD move(style_)),
    _uvs({
        math::vec2 { 0.F, 0.F },
        math::vec2 { 1.F, 0.F },
        math::vec2 { 1.F, 1.F },
        math::vec2 { 0.F, 1.F }
    }),
    _imageResource(nullptr) {}

Image::~Image() {
    // Warning: Workaround which might cause an crash or error
    if (_imageResource) {
        _imageResource = nullptr;
        _image = nullptr;
    }
}

void Image::setImage(cref<decltype(_image)> image_, ptr<void> resource_) {
    _image = image_;
    _imageResource = resource_;
}

void Image::render(const ptr<ReflowCommandBuffer> cmd_) {

    if (not _computedStyle.borderRadius->zero()) {
        return;
    }

    const auto off {
        _screenOff + math::vec2 {
            _computedStyle.margin->x,
            _computedStyle.margin->y
        }
    };

    gfx::ProxyTexture<non_owning_rptr> proxy { *_image };

    cmd_->drawImage(
        math::vec2 { off.x, off.y }, _uvs[0],
        math::vec2 { off.x + _innerSize.x, off.y }, _uvs[1],
        math::vec2 { off.x + _innerSize.x, off.y + _innerSize.y }, _uvs[2],
        math::vec2 { off.x, off.y + _innerSize.y }, _uvs[3],
        _STD move(proxy),
        _computedStyle.color
    );
}

void Image::flow(cref<FlowContext> ctx_, cref<math::vec2> space_, ref<StyleKeyStack> styleStack_) {

    styleStack_.pushLayer();
    _computedStyle = _style->compute(shared_from_this(), styleStack_);

    const bool autoWidth { _computedStyle.width->type == ReflowUnitType::eAuto };
    const bool autoHeight { _computedStyle.height->type == ReflowUnitType::eAuto };

    /**/

    math::vec2 local { 0.F };

    if (_computedStyle.width->type == ReflowUnitType::eRelative) {
        local.x = _computedStyle.width->value * space_.x;
    } else if (_computedStyle.width->type == ReflowUnitType::eAbsolute) {
        local.x = _computedStyle.width->value;
    }

    if (_computedStyle.height->type == ReflowUnitType::eRelative) {
        local.y = _computedStyle.height->value * space_.y;
    } else if (_computedStyle.height->type == ReflowUnitType::eAbsolute) {
        local.y = _computedStyle.height->value;
    }

    /**/

    if (_computedStyle.minWidth->type == ReflowUnitType::eRelative) {
        local.x = MAX(local.x, _computedStyle.minWidth->value * space_.x);
    } else if (_computedStyle.minWidth->type == ReflowUnitType::eAbsolute) {
        local.x = MAX(local.x, _computedStyle.minWidth->value);
    }

    if (_computedStyle.minHeight->type == ReflowUnitType::eRelative) {
        local.y = MAX(local.y, _computedStyle.minHeight->value * space_.y);
    } else if (_computedStyle.minHeight->type == ReflowUnitType::eAbsolute) {
        local.y = MAX(local.y, _computedStyle.minHeight->value);
    }

    /**/

    if (_computedStyle.maxWidth->type == ReflowUnitType::eRelative) {
        local.x = MIN(local.x, _computedStyle.maxWidth->value * space_.x);
    } else if (_computedStyle.maxWidth->type == ReflowUnitType::eAbsolute) {
        local.x = MIN(local.x, _computedStyle.maxWidth->value);
    }

    if (_computedStyle.maxHeight->type == ReflowUnitType::eRelative) {
        local.y = MIN(local.y, _computedStyle.maxHeight->value * space_.y);
    } else if (_computedStyle.maxHeight->type == ReflowUnitType::eAbsolute) {
        local.y = MIN(local.y, _computedStyle.maxHeight->value);
    }

    /**/

    _innerSize = local;
    styleStack_.popLayer();

    //clearPending();
}

void Image::shift(cref<FlowContext> ctx_, cref<math::vec2> offset_) {
    _screenOff = offset_;
}

math::vec2 Image::outerSize() const noexcept {

    auto size { innerSize() };

    if (not _computedStyle.margin->zero()) {
        size.x += _computedStyle.margin->x;
        size.x += _computedStyle.margin->z;
        size.y += _computedStyle.margin->y;
        size.y += _computedStyle.margin->w;
    }

    return size;
}

math::vec2 Image::innerSize() const noexcept {
    return _innerSize;
}

math::vec2 Image::screenOffset() const noexcept {
    return _screenOff;
}
