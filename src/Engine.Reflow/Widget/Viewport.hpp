#pragma once

#include <Engine.GFX/Swapchain/__fwd.hpp>

#include "Widget.hpp"
#include "../Children.hpp"
#include "../ReflowUnit.hpp"

namespace hg {
    class CameraActor;
}

namespace hg::engine::reflow {
    class Viewport :
        public Widget {
    public:
        using this_type = Viewport;

    public:
        Viewport();

        ~Viewport() override;

    public:
        [[nodiscard]] string getTag() const noexcept override;

    public:
        struct Attributes {
            Attribute<ReflowUnit> minWidth;
            Attribute<ReflowUnit> width;
            Attribute<ReflowUnit> maxWidth;

            Attribute<ReflowUnit> minHeight;
            Attribute<ReflowUnit> height;
            Attribute<ReflowUnit> maxHeight;
        } attr;

    private:
        void tidy();

    private:
        uptr<gfx::VkSwapchain> _swapchain;
        ptr<CameraActor> _cameraActor;

    public:
        [[nodiscard]] const non_owning_rptr<gfx::Swapchain> getSwapchain() const noexcept;

        [[nodiscard]] const non_owning_rptr<CameraActor> getCameraActor() const noexcept;

        void setCameraActor(const ptr<CameraActor> actor_);

    private:
        _STD array<math::vec2, 4> _uvs;

        math::uivec2 _viewSize;
        Vector<_STD function<void(const ptr<gfx::VkSwapchain>)>> _viewListen;

    private:
        [[nodiscard]] math::uivec2 actualViewExtent() const noexcept;

        [[nodiscard]] bool viewHasChanged() const noexcept;

        void handleViewListener(const ptr<gfx::VkSwapchain> next_);

    public:
        void rebuildView();

        void resizeView(cref<math::uivec2> extent_);

        void addResizeListener(mref<_STD function<void(const ptr<gfx::VkSwapchain>)>> fnc_);

        void removeResizeListener();

    public:
        const ptr<const NullChildren> children() const override;

    public:
        void render(const ptr<ReflowCommandBuffer> cmd_) override;

    public:
        math::vec2 prefetchDesiredSize(cref<ReflowState> state_, float scale_) const override;

        math::vec2 computeDesiredSize(cref<ReflowPassState> passState_) const override;

        void applyLayout(ref<ReflowState> state_, mref<LayoutContext> ctx_) override;

    public:
        EventResponse onFocus(cref<FocusEvent> event_) override;

        EventResponse onBlur(cref<FocusEvent> event_) override;

        EventResponse onKeyDown(cref<KeyboardEvent> event_) override;

        EventResponse onKeyUp(cref<KeyboardEvent> event_) override;

        EventResponse onMouseEnter(cref<MouseMoveEvent> event_) override;

        EventResponse onMouseMove(cref<MouseMoveEvent> event_) override;

        EventResponse onMouseLeave(cref<MouseMoveEvent> event_) override;
    };
}
