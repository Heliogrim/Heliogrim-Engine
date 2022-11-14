#include "Viewport.hpp"

#include "Engine.GFX/Swapchain/VkSwapchain.hpp"
#include "Engine.Reflow/Style/BoundStyleSheet.hpp"

#if TRUE
#include <Engine.Logging/Logger.hpp>
#include <Engine.Session/Session.hpp>
#include <Engine.GFX/Graphics.hpp>
#include <Engine.GFX/Device/Device.hpp>
#include "Engine.GFX/RenderTarget.hpp"
#include "Engine.GFX/Camera/Camera.hpp"
#include <Engine.GFX/Texture/ProxyTexture.hpp>
#endif

using namespace ember::engine::reflow;
using namespace ember;

Viewport::Viewport(mref<sptr<BoundStyleSheet>> style_) :
    Widget(),
    _style(style_),
    _swapchain(nullptr),
    _uvs({
        math::vec2 { 0.F, 0.F },
        math::vec2 { 1.F, 0.F },
        math::vec2 { 1.F, 1.F },
        math::vec2 { 0.F, 1.F }
    }),
    _viewSize(),
    _viewListen() {}

Viewport::~Viewport() {
    tidy();
}

string Viewport::getTag() const noexcept {
    return _STD format(R"(Viewport <{:#x}>)", reinterpret_cast<u64>(this));
}

void Viewport::tidy() {

    if (_swapchain) {
        _swapchain->destroy();

        delete _swapchain;
        _swapchain = nullptr;
    }

    /**/

    _viewListen.clear();
    _viewListen.shrink_to_fit();
}

math::uivec2 Viewport::actualViewExtent() const noexcept {

    if (_viewSize.zero()) {
        return math::uivec2 {
            static_cast<u32>(_innerSize.x),
            static_cast<u32>(_innerSize.y)
        };
    }

    return _viewSize;
}

bool Viewport::viewHasChanged() const noexcept {

    if (!_swapchain) {
        return true;
    }

    const auto size { actualViewExtent() };
    return _swapchain->extent() != size;
}

void Viewport::rebuildView() {

    const auto extent { actualViewExtent() };
    auto* nextSwapchain { make_ptr<gfx::VkSwapchain>() };

    nextSwapchain->setExtent(extent);
    nextSwapchain->setFormat(gfx::TextureFormat::eB8G8R8A8Unorm);
    nextSwapchain->setDesiredImages(2ui32);

    const auto device { engine::Session::get()->modules().graphics()->getCurrentDevice() };

    nextSwapchain->setup(device);

    /**/

    handleViewListener(nextSwapchain);
    auto* prevSwapchain { _STD exchange(_swapchain, nextSwapchain) };

    /**/

    if (prevSwapchain) {
        prevSwapchain->destroy();
        delete prevSwapchain;
    }

}

void Viewport::handleViewListener(const ptr<gfx::VkSwapchain> next_) {
    for (const auto& fnc : _viewListen) {
        fnc(next_);
    }
}

void Viewport::resizeView(cref<math::uivec2> extent_) {
    _viewSize = extent_;
}

void Viewport::addResizeListener(mref<std::function<void(const ptr<gfx::VkSwapchain>)>> fnc_) {
    _viewListen.push_back(_STD move(fnc_));
}

void Viewport::removeResizeListener() {
    __debugbreak();
}

void Viewport::render(const ptr<ReflowCommandBuffer> cmd_) {

    if (_innerSize.zero()) {
        return;
    }

    if (viewHasChanged()) {

        rebuildView();

        auto gfx { engine::Session::get()->modules().graphics() };
        gfx->_secondarySwapchain = _swapchain;

        gfx->_camera->setAspect(_swapchain->extent().x / _swapchain->extent().y);

        if (gfx->_renderTarget->ready()) {
            gfx->_renderTarget->rebuildPasses(_swapchain);
        } else {
            gfx->_renderTarget->use(_swapchain);
            gfx->_renderTarget->buildPasses(gfx->_camera);
        }

    }

    /**/

    if (not _computedStyle.borderRadius.attr.zero()) {
        return;
    }

    const auto off {
        _screenOff + math::vec2 {
            _computedStyle.margin.attr.x,
            _computedStyle.margin.attr.y
        }
    };

    sptr<gfx::Texture> image {};
    vk::Semaphore imageSignal {};
    Vector<vk::Semaphore> imageWaits {};

    const auto result { _swapchain->consumeNext(image, imageSignal, imageWaits) };
    if (!result) {
        IM_CORE_ERROR("Skipping viewporw draw due to missing next swapchain frame.");
    }

    if (result) {

        gfx::ProxyTexture<non_owning_rptr> proxy { _STD move(image.get()) };

        cmd_->drawImageAsync(
            math::vec2 { off.x, off.y }, _uvs[0],
            math::vec2 { off.x + _innerSize.x, off.y }, _uvs[1],
            math::vec2 { off.x + _innerSize.x, off.y + _innerSize.y }, _uvs[2],
            math::vec2 { off.x, off.y + _innerSize.y }, _uvs[3],
            _STD move(proxy),
            imageWaits.empty() ? VK_NULL_HANDLE : imageWaits.back(),
            imageSignal,
            _computedStyle.color.attr
        );

    }

}

void Viewport::flow(cref<FlowContext> ctx_, cref<math::vec2> space_, cref<math::vec2> limit_,
    ref<StyleKeyStack> styleStack_) {

    styleStack_.pushLayer();
    _computedStyle = _style->compute(shared_from_this(), styleStack_);
    const auto& style { _computedStyle };

    const bool autoWidth { style.width.attr.type == ReflowUnitType::eAuto };
    const bool autoHeight { style.height.attr.type == ReflowUnitType::eAuto };

    /**/

    math::vec2 maxSize { limit_ };

    if (_computedStyle.maxWidth->type != ReflowUnitType::eAuto) {
        if (_computedStyle.maxWidth->type == ReflowUnitType::eRelative) {
            maxSize.x = MIN(maxSize.x,
                MAX(_computedStyle.maxWidth->value * space_.x - (_computedStyle.padding->x + _computedStyle.padding->z),
                    0));
        } else if (_computedStyle.maxWidth->type == ReflowUnitType::eAbsolute) {
            maxSize.x = MIN(maxSize.x,
                MAX(_computedStyle.maxWidth->value - (_computedStyle.padding->x + _computedStyle.padding->z), 0));
        }
    }

    if (_computedStyle.maxHeight->type != ReflowUnitType::eAuto) {
        if (_computedStyle.maxHeight->type == ReflowUnitType::eRelative) {
            maxSize.y = MIN(maxSize.y,
                MAX(_computedStyle.maxHeight->value * space_.y - (_computedStyle.padding->y + _computedStyle.padding->w)
                    , 0));
        } else if (_computedStyle.maxHeight->type == ReflowUnitType::eAbsolute) {
            maxSize.y = MIN(maxSize.y,
                MAX(_computedStyle.maxHeight->value - (_computedStyle.padding->y + _computedStyle.padding->w), 0));
        }
    }

    /**/

    math::vec2 local { 0.F };

    if (style.width.attr.type == ReflowUnitType::eRelative) {
        local.x = style.width.attr.value * space_.x;
    } else if (style.width.attr.type == ReflowUnitType::eAbsolute) {
        local.x = style.width.attr.value;
    }

    if (style.height.attr.type == ReflowUnitType::eRelative) {
        local.y = style.height.attr.value * space_.y;
    } else if (style.height.attr.type == ReflowUnitType::eAbsolute) {
        local.y = style.height.attr.value;
    }

    /**/

    if (style.minWidth.attr.type == ReflowUnitType::eRelative) {
        local.x = MAX(local.x, style.minWidth.attr.value * space_.x);
    } else if (style.minWidth.attr.type == ReflowUnitType::eAbsolute) {
        local.x = MAX(local.x, style.minWidth.attr.value);
    }

    if (style.minHeight.attr.type == ReflowUnitType::eRelative) {
        local.y = MAX(local.y, style.minHeight.attr.value * space_.y);
    } else if (style.minHeight.attr.type == ReflowUnitType::eAbsolute) {
        local.y = MAX(local.y, style.minHeight.attr.value);
    }

    /**/

    local = math::compMin<float>(local, maxSize);

    /**/

    _innerSize = local;
    styleStack_.popLayer();
}

void Viewport::shift(cref<FlowContext> ctx_, cref<math::vec2> offset_) {
    _screenOff = offset_;
}

math::vec2 Viewport::outerSize() const noexcept {

    auto size { innerSize() };

    if (not _computedStyle.margin.attr.zero()) {
        size.x += _computedStyle.margin.attr.x;
        size.x += _computedStyle.margin.attr.z;
        size.y += _computedStyle.margin.attr.y;
        size.y += _computedStyle.margin.attr.w;
    }

    return size;
}

math::vec2 Viewport::innerSize() const noexcept {
    return _innerSize;
}

math::vec2 Viewport::screenOffset() const noexcept {
    return _screenOff;
}
