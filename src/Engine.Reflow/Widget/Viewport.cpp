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
#include <Engine.Common/Math/Coordinates.hpp>
#include <Engine.GFX/Texture/ProxyTexture.hpp>
#include <Ember/Actors/CameraActor.hpp>
#endif

using namespace ember::engine::reflow;
using namespace ember;

Viewport::Viewport(mref<sptr<BoundStyleSheet>> style_) :
    Widget(),
    _style(style_),
    _swapchain(nullptr),
    _cameraActor(nullptr),
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
        _swapchain.reset();
    }

    /**/

    _viewListen.clear();
    _viewListen.shrink_to_fit();
}

const non_owning_rptr<engine::gfx::Swapchain> Viewport::getSwapchain() const noexcept {
    return _swapchain.get();
}

const non_owning_rptr<CameraActor> Viewport::getCameraActor() const noexcept {
    return _cameraActor;
}

void Viewport::setCameraActor(const ptr<CameraActor> actor_) {
    _cameraActor = actor_;
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
    auto nextSwapchain { make_uptr<gfx::VkSwapchain>() };

    nextSwapchain->setExtent(extent);
    nextSwapchain->setFormat(gfx::TextureFormat::eB8G8R8A8Unorm);
    nextSwapchain->setDesiredImages(2ui32);

    const auto device { engine::Session::get()->modules().graphics()->getCurrentDevice() };

    nextSwapchain->setup(device);

    /**/

    handleViewListener(nextSwapchain.get());

    /**/

    if (_swapchain) {
        _swapchain->destroy();
    }
    _swapchain = _STD move(nextSwapchain);
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
        gfx->_secondarySwapchain = _swapchain.get();

        if (not _cameraActor) {
            // TODO: Most likely generate a new camera actor
        }

        _cameraActor->getCameraComponent()->setAspectRatio(
            static_cast<float>(_swapchain->extent().x) / static_cast<float>(_swapchain->extent().y)
        );

        /*
        if (gfx->_renderTarget->ready()) {
            gfx->_renderTarget->rebuildPasses(_swapchain.get());
        } else {
            gfx->_renderTarget->use(_swapchain.get());
            gfx->_renderTarget->buildPasses(_camera.get());
        }
         */

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
        IM_CORE_ERROR("Skipping viewport draw due to missing next swapchain frame.");
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

EventResponse Viewport::onFocus(cref<FocusEvent> event_) {
    _state.set(WidgetStateFlagBits::eFocus);
    markAsPending();
    return EventResponse::eConsumed;
}

EventResponse Viewport::onBlur(cref<FocusEvent> event_) {
    _state.unset(WidgetStateFlagBits::eFocus);
    markAsPending();
    return EventResponse::eConsumed;
}

EventResponse Viewport::onKeyDown(cref<KeyboardEvent> event_) {

    // Early exit on ESC Key to drop focus and ctrl
    if (event_._key == '\x1B') {
        _state.unset(WidgetStateFlagBits::eFocus);
        markAsPending();
        return EventResponse::eConsumed;
    }

    const auto isShift { (event_._modifier & 0x3) != 0x0 };
    const float factor { 0.1 };

    auto response { EventResponse::eHandled };
    switch (event_._key) {
        case 'a': {
            response = EventResponse::eConsumed;

            math::mat4 rotation { 1.F };
            cref<Transform> tf { _cameraActor->getWorldTransform() };

            const_cast<ref<Transform>>(tf).setPosition(
                tf.position() + (rotation * math::vec4 { math::vec3_left, 0.F }).xyz().normalize() * factor
            );
            break;
        }
        case 'd': {
            response = EventResponse::eConsumed;

            math::mat4 rotation { 1.F };
            cref<Transform> tf { _cameraActor->getWorldTransform() };

            const_cast<ref<Transform>>(tf).setPosition(
                tf.position() + (rotation * math::vec4 { math::vec3_right, 0.F }).xyz().normalize() * factor
            );
            break;
        }
        case 's': {
            response = EventResponse::eConsumed;

            math::mat4 rotation { 1.F };
            cref<Transform> tf { _cameraActor->getWorldTransform() };

            const_cast<ref<Transform>>(tf).setPosition(
                tf.position() + (rotation * math::vec4 { math::vec3_backward, 0.F }).xyz().normalize() * factor
            );
            break;
        }
        case 'w': {
            response = EventResponse::eConsumed;

            math::mat4 rotation { 1.F };
            cref<Transform> tf { _cameraActor->getWorldTransform() };

            const_cast<ref<Transform>>(tf).setPosition(
                tf.position() + (rotation * math::vec4 { math::vec3_forward, 0.F }).xyz().normalize() * factor
            );
            break;
        }
        case 'q': {
            response = EventResponse::eConsumed;

            math::mat4 rotation { 1.F };
            cref<Transform> tf { _cameraActor->getWorldTransform() };

            const_cast<ref<Transform>>(tf).setPosition(
                tf.position() + (rotation * math::vec4 { math::vec3_up, 0.F }).xyz().normalize() * factor
            );
            break;
        }

        case 'c': {
            response = EventResponse::eConsumed;

            math::mat4 rotation { 1.F };
            cref<Transform> tf { _cameraActor->getWorldTransform() };

            const_cast<ref<Transform>>(tf).setPosition(
                tf.position() + (rotation * math::vec4 { math::vec3_down, 0.F }).xyz().normalize() * factor
            );
            break;
        }
        default: { }
    }

    return response;
}

EventResponse Viewport::onKeyUp(cref<KeyboardEvent> event_) {
    return Widget::onKeyUp(event_);
}

EventResponse Viewport::onMouseEnter(cref<MouseMoveEvent> event_) {
    return Widget::onMouseEnter(event_);
}

EventResponse Viewport::onMouseMove(cref<MouseMoveEvent> event_) {

    const auto isShift { (event_._modifier & 0x3) != 0x0 };
    const auto rbtn { (event_._button & 0x4) != 0x0 };

    if (!rbtn) {
        return EventResponse::eConsumed;
    }

    constexpr float deltaTime { (1000.F / 60.F) / 1000.F };
    constexpr float speed { 4.F };

    math::vec2 dxdy {
        math::vec2 {
            static_cast<float>(event_._delta.x),
            static_cast<float>(event_._delta.y)
        }
        * speed
        * deltaTime
    };

    dxdy.x = glm::radians(dxdy.x);
    dxdy.y = glm::radians(dxdy.y);

    const auto pitch { math::quaternion::euler({ -dxdy.y, 0.F, 0.F }) };
    const auto yaw { math::quaternion::euler({ 0.F, dxdy.x, 0.F }) };

    cref<Transform> rtf { _cameraActor->getRootComponent()->getWorldTransform() };
    cref<Transform> ctf { _cameraActor->getCameraComponent()->getWorldTransform() };

    const_cast<ref<Transform>>(rtf).setRotation(rtf.rotation() * yaw);
    const_cast<ref<Transform>>(ctf).setRotation(ctf.rotation() * pitch);

    return EventResponse::eConsumed;
}

EventResponse Viewport::onMouseLeave(cref<MouseMoveEvent> event_) {
    return Widget::onMouseLeave(event_);
}
