#pragma once

#include <Engine.GFX/Camera/Camera.hpp>
#include <Engine.GFX/Swapchain/__fwd.hpp>

#include "Widget.hpp"
#include "../Style/__fwd.hpp"
#include "../Style/StyleSheet.hpp"

namespace ember::engine::reflow {

    class Viewport :
        public Widget {
    public:
        using this_type = Viewport;

    public:
        Viewport(mref<sptr<BoundStyleSheet>> style_);

        ~Viewport() override;

    public:
        [[nodiscard]] string getTag() const noexcept override;

    private:
        void tidy();

    private:
        sptr<BoundStyleSheet> _style;
        StyleSheet _computedStyle;

    private:
        uptr<gfx::VkSwapchain> _swapchain;
        sptr<gfx::Camera> _camera;

    public:
        [[nodiscard]] const non_owning_rptr<gfx::Swapchain> getSwapchain() const noexcept;

        [[nodiscard]] const non_owning_rptr<gfx::Camera> getCamera() const noexcept;

        void setCamera(cref<sptr<gfx::Camera>> camera_);

    private:
        _STD array<math::vec2, 4> _uvs;

        math::uivec2 _viewSize;
        Vector<_STD function<void(const ptr<gfx::VkSwapchain>)>> _viewListen;

    private:
        [[nodiscard]] math::uivec2 actualViewExtent() const noexcept;

        [[nodiscard]] bool viewHasChanged() const noexcept;

        void rebuildView();

        void handleViewListener(const ptr<gfx::VkSwapchain> next_);

    public:
        void resizeView(cref<math::uivec2> extent_);

        void addResizeListener(mref<_STD function<void(const ptr<gfx::VkSwapchain>)>> fnc_);

        void removeResizeListener();

    public:
        void render(const ptr<ReflowCommandBuffer> cmd_) override;

        void flow(cref<FlowContext> ctx_, cref<math::vec2> space_, cref<math::vec2> limit_,
            ref<StyleKeyStack> styleStack_) override;

        void shift(cref<FlowContext> ctx_, cref<math::vec2> offset_) override;

    private:
        math::vec2 _innerSize;
        math::vec2 _screenOff;

    public:
        [[nodiscard]] math::vec2 outerSize() const noexcept override;

        [[nodiscard]] math::vec2 innerSize() const noexcept override;

        [[nodiscard]] math::vec2 screenOffset() const noexcept override;
    };

}
