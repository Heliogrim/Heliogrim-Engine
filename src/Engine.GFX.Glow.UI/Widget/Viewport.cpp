#include "Viewport.hpp"

#include "Engine.GFX/RenderTarget.hpp"
#include "Engine.GFX/Camera/Camera.hpp"

#if TRUE
#include <Engine.Session/Session.hpp>
#include <Engine.GFX/Graphics.hpp>
#endif

using namespace ember::engine::gfx::glow::ui;
using namespace ember;

Viewport::Viewport() :
    Widget(),
    _swapchain(nullptr),
    _uvs({
        math::vec2 { 0.F, 0.F },
        math::vec2 { 1.F, 0.F },
        math::vec2 { 1.F, 1.F },
        math::vec2 { 0.F, 1.F }
    }),
    _viewportExtent(math::uivec2 {}),
    _viewportCallback() {}

Viewport::~Viewport() {
    tidy();
}

void Viewport::tidy() {

    /**/
    if (_swapchain) {
        _swapchain->destroy();

        delete _swapchain;
        _swapchain = nullptr;
    }

    /**/
    _viewportCallback.clear();
    _viewportCallback.shrink_to_fit();
}

void Viewport::render(const ptr<UICommandBuffer> cmd_) {

    {
        if (viewportHasChanged()) {
            rebuildViewport();

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
    }

    const color cl { 255.F, 255.F, 255.F, 255.F };

    const auto& tf { _transform };
    constexpr math::vec4 borderRadius { 0.F };

    if (borderRadius.zero()) {

        const math::vec2 c0 { tf.offsetX, tf.offsetY };
        const math::vec2 c1 { tf.offsetX + tf.width, tf.offsetY };
        const math::vec2 c2 { tf.offsetX + tf.width, tf.offsetY + tf.height };
        const math::vec2 c3 { tf.offsetX, tf.offsetY + tf.height };

        /*
        cmd_->drawImage(
            c0, _uvs[0],
            c1, _uvs[1],
            c2, _uvs[2],
            c3, _uvs[3],
            _image,
            cl
        );
         */

        sptr<Texture> image {};
        vk::Semaphore imageSignal {};
        Vector<vk::Semaphore> imageWaits {};

        const auto result { _swapchain->consumeNext(image, imageSignal, imageWaits) };
        if (!result) {
            DEBUG_SNMSG(false, "GFX", "Skipping viewporw draw due to missing next swapchain frame.")
        }

        if (result) {

            cmd_->drawImageAsync(
                c0, _uvs[0],
                c1, _uvs[1],
                c2, _uvs[2],
                c3, _uvs[3],
                ProxyTexture<non_owning_rptr> { _STD move(image.get()) },
                imageWaits.empty() ? VK_NULL_HANDLE : imageWaits.back(),
                imageSignal,
                cl
            );

        }
    }

    for (const auto& widget : _nodes) {
        widget->render(cmd_);
    }

}

math::uivec2 Viewport::effectiveViewportExtent() const noexcept {
    if (_viewportExtent.zero()) {
        return math::uivec2 {
            static_cast<u32>(_transform.width),
            static_cast<u32>(_transform.height)
        };
    }

    return _viewportExtent;
}

bool Viewport::viewportHasChanged() const noexcept {
    if (!_swapchain) {
        return true;
    }

    const auto extent { effectiveViewportExtent() };
    return _swapchain->extent() != extent;
}

#if TRUE
#include <Engine.Session/Session.hpp>
#include <Engine.GFX/Graphics.hpp>
#include <Engine.GFX/Device/Device.hpp>
#endif

void Viewport::rebuildViewport() {

    const auto extent { effectiveViewportExtent() };
    auto* nextSwapchain { make_ptr<VkSwapchain>() };

    nextSwapchain->setExtent(extent);
    nextSwapchain->setFormat(TextureFormat::eB8G8R8A8Unorm);
    nextSwapchain->setDesiredImages(2ui32);

    const auto device { engine::Session::get()->modules().graphics()->getCurrentDevice() };

    nextSwapchain->setup(device);

    /**/

    handleViewportCallbacks(nextSwapchain);
    auto* prevSwapchain { _STD exchange(_swapchain, nextSwapchain) };

    /**/

    if (prevSwapchain) {
        prevSwapchain->destroy();
        delete prevSwapchain;
    }
}

void Viewport::handleViewportCallbacks(const ptr<VkSwapchain> next_) {
    for (const auto& fnc : _viewportCallback) {
        fnc(next_);
    }
}

const ptr<engine::gfx::VkSwapchain> Viewport::swapchain() const noexcept {
    return _swapchain;
}

void Viewport::resizeViewport(cref<math::uivec2> extent_) {
    _viewportExtent = extent_;
}

void Viewport::addResizeCallback(mref<std::function<void(const ptr<VkSwapchain> swapchain_)>> callback_) {
    _viewportCallback.push_back(_STD move(callback_));
}

void Viewport::removeResizeCallback(mref<std::function<void(const ptr<VkSwapchain> swapchain_)>> callback_) {
    /*
    const auto where { _STD remove(_viewportCallback.begin(), _viewportCallback.end(), callback_) };
    _viewportCallback.erase(where, _viewportCallback.end());
     */
}
