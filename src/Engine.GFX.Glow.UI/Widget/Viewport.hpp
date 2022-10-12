#pragma once
#include <Engine.GFX/Swapchain/VkSwapchain.hpp>

#include "Widget.hpp"

namespace ember::engine::gfx::glow::ui {

    class Viewport : public Widget {
    public:
        Viewport();

        ~Viewport() override;

    private:
        void tidy();

    public:
        void render(const ptr<UICommandBuffer> cmd_) override;

        math::vec2 reflow(cref<math::vec2> available_, cref<math::vec2> reflow_) override {
            return math::vec2 { 0.375F, 0.4444444F } + reflow_;
        }

    private:
        [[nodiscard]] math::uivec2 effectiveViewportExtent() const noexcept;

        [[nodiscard]] bool viewportHasChanged() const noexcept;

        void rebuildViewport();

        void handleViewportCallbacks(const ptr<VkSwapchain> next_);

    private:
        ptr<VkSwapchain> _swapchain;
    public:
        _STD array<math::vec2, 4> _uvs;

    public:
        [[nodiscard]] const ptr<VkSwapchain> swapchain() const noexcept;

    private:
        math::uivec2 _viewportExtent;

        Vector<_STD function<void(const ptr<VkSwapchain>)>> _viewportCallback;

    public:
        void resizeViewport(cref<math::uivec2> extent_);

        void addResizeCallback(mref<_STD function<void(const ptr<VkSwapchain> swapchain_)>> callback_);

        void removeResizeCallback(mref<_STD function<void(const ptr<VkSwapchain> swapchain_)>> callback_);
    };

}
