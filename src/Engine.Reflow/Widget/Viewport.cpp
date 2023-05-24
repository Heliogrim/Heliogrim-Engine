#include "Viewport.hpp"

#include "Engine.Common/Math/Convertion.hpp"
#include "Engine.GFX/Swapchain/VkSwapchain.hpp"

#if TRUE
#include <Engine.Logging/Logger.hpp>
#include <Engine.GFX/Graphics.hpp>
#include "Engine.GFX/RenderTarget.hpp"
#include <Engine.Common/Math/Coordinates.hpp>
#include <Engine.GFX/Texture/ProxyTexture.hpp>
#include <Heliogrim/Actors/CameraActor.hpp>
#include <Engine.Core/Engine.hpp>
#endif

#include "../Layout/Style.hpp"

using namespace hg::engine::reflow;
using namespace hg;

Viewport::Viewport() :
    Widget(),
    attr(
        Attributes {
            .minWidth = { this, { ReflowUnitType::eAuto, 0.F } },
            .width = { this, { ReflowUnitType::eAuto, 0.F } },
            .maxWidth = { this, { ReflowUnitType::eAuto, 0.F } },
            .minHeight = { this, { ReflowUnitType::eAuto, 0.F } },
            .height = { this, { ReflowUnitType::eAuto, 0.F } },
            .maxHeight = { this, { ReflowUnitType::eAuto, 0.F } }
        }
    ),
    _swapchain(nullptr),
    _cameraActor(nullptr),
    _uvs(
        {
            math::vec2 { 0.F, 0.F },
            math::vec2 { 1.F, 0.F },
            math::vec2 { 1.F, 1.F },
            math::vec2 { 0.F, 1.F }
        }
    ),
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
            static_cast<u32>(_layoutState.layoutSize.x),
            static_cast<u32>(_layoutState.layoutSize.y)
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

    const auto device { Engine::getEngine()->getGraphics()->getCurrentDevice() };

    nextSwapchain->setup(device);

    /**/

    if (_cameraActor) {
        auto* const cc { _cameraActor->getCameraComponent() };
        cc->setAspectRatio(
            static_cast<float>(nextSwapchain->extent().x) / static_cast<float>(nextSwapchain->extent().y)
        );
    }

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

const ptr<const NullChildren> Viewport::children() const {
    return get_null_children();
}

void Viewport::render(const ptr<ReflowCommandBuffer> cmd_) {

    if (_layoutState.layoutSize.zero()) {
        return;
    }

    if (viewHasChanged()) {
        rebuildView();

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

    const auto offset = _layoutState.layoutOffset;
    const auto size = _layoutState.layoutSize;

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
            math::vec2 { offset.x, offset.y },
            _uvs[0],
            math::vec2 { offset.x + size.x, offset.y },
            _uvs[1],
            math::vec2 { offset.x + size.x, offset.y + size.y },
            _uvs[2],
            math::vec2 { offset.x, offset.y + size.y },
            _uvs[3],
            _STD move(proxy),
            reinterpret_cast<_::VkSemaphore>(imageWaits.empty() ? nullptr : imageWaits.back().operator VkSemaphore()),
            reinterpret_cast<_::VkSemaphore>(imageSignal.operator VkSemaphore()),
            engine::color { 255.F, 255.F, 255.F, 255.F }
        );
    }
}

math::vec2 Viewport::prefetchDesiredSize(cref<ReflowState> state_, float scale_) const {

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

math::vec2 Viewport::computeDesiredSize(cref<ReflowPassState> passState_) const {

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

void Viewport::applyLayout(ref<ReflowState> state_, mref<LayoutContext> ctx_) {}

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

    if (not _cameraActor) {
        return EventResponse::eHandled;
    }

    const auto isShift { (event_._modifier & 0x3) != 0x0 };
    const float factor { 0.1F };

    auto response { EventResponse::eHandled };
    switch (event_._key) {
        case 'a': {
            response = EventResponse::eConsumed;

            ref<Transform> rtf { _cameraActor->getRootComponent()->getLocalTransform() };

            auto rlwd = rtf.rotator().quaternion()._left();

            rtf.setLocation(
                math::Location(
                    rtf.location().operator math::fvec3() + rlwd.normalize() * factor
                )
            );
            break;
        }
        case 'd': {
            response = EventResponse::eConsumed;

            ref<Transform> rtf { _cameraActor->getRootComponent()->getLocalTransform() };

            auto rrwd = -rtf.rotator().quaternion()._left();

            rtf.setLocation(
                math::Location(
                    rtf.location().operator math::fvec3() + rrwd.normalize() * factor
                )
            );
            break;
        }
        case 's': {
            response = EventResponse::eConsumed;

            ref<Transform> rtf { _cameraActor->getRootComponent()->getLocalTransform() };
            cref<Transform> ctf = _cameraActor->getCameraComponent()->getLocalTransform();

            const auto rotator = math::Rotator::combine(rtf.rotator(), ctf.rotator());
            auto rbwd = -rotator.quaternion().__forward();
            rbwd.x *= -1.F;

            rtf.setLocation(
                math::Location(
                    rtf.location().operator math::fvec3() + rbwd.normalize() * factor
                )
            );
            break;
        }
        case 'w': {
            response = EventResponse::eConsumed;

            ref<Transform> rtf { _cameraActor->getRootComponent()->getLocalTransform() };
            cref<Transform> ctf = _cameraActor->getCameraComponent()->getLocalTransform();

            const auto rotator = math::Rotator::combine(rtf.rotator(), ctf.rotator());
            auto rfwd = rotator.quaternion().__forward();
            rfwd.x *= -1.F;

            rtf.setLocation(
                math::Location(
                    rtf.location().operator math::fvec3() + rfwd.normalize() * factor
                )
            );
            break;
        }
        case 'q': {
            response = EventResponse::eConsumed;

            ref<Transform> tf { _cameraActor->getRootComponent()->getLocalTransform() };

            tf.setLocation(
                math::Location(
                    tf.location().operator math::fvec3() + math::vec3_down * factor
                )
            );
            break;
        }

        case 'c': {
            response = EventResponse::eConsumed;

            ref<Transform> tf { _cameraActor->getRootComponent()->getLocalTransform() };

            tf.setLocation(
                math::Location(
                    tf.location().operator math::fvec3() + math::vec3_up * factor
                )
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

    if (!rbtn || not _cameraActor) {
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

    ref<Transform> rtf { _cameraActor->getRootComponent()->getLocalTransform() };
    ref<Transform> ctf { _cameraActor->getCameraComponent()->getLocalTransform() };

    rtf.rotator() += math::Rotator { 0.F, dxdy.x, 0.F };
    ctf.rotator() += math::Rotator { dxdy.y, 0.F, 0.F };

    return EventResponse::eConsumed;
}

EventResponse Viewport::onMouseLeave(cref<MouseMoveEvent> event_) {
    return Widget::onMouseLeave(event_);
}
