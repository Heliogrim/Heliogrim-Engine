#pragma once

#include "Widget.hpp"
#include "../Children.hpp"

namespace hg::engine::reflow {
    class Overlay :
        public Widget {
    public:
        using this_type = Overlay;

    public:
        Overlay();

        ~Overlay() override;

    public:
        [[nodiscard]] string getTag() const noexcept override;

    private:
        SingleChildren _children;

    public:
        [[nodiscard]] const ptr<const Children> children() const override;

    public:
        [[nodiscard]] sptr<Widget> getContent() const noexcept;

        void setContent(cref<sptr<Widget>> content_);

    public:
        void render(cref<ReflowState> state_, const ptr<ReflowCommandBuffer> cmd_) override;

    public:
        [[nodiscard]] ReflowPosition position() const noexcept override;

    public:
        math::vec2 prefetchDesiredSize(cref<ReflowState> state_, float scale_) const override;

        math::vec2 computeDesiredSize(cref<ReflowPassState> passState_) const override;

        void applyLayout(ref<ReflowState> state_, mref<LayoutContext> ctx_) override;
    };
}
