#pragma once

#include "Widget.hpp"

namespace hg::engine::reflow {
    class NullWidget final :
        public Widget {
    public:
        using this_type = NullWidget;

    protected:
        NullWidget();

    public:
        ~NullWidget() override;

    public:
        static sptr<NullWidget> instance();

    public:
        [[nodiscard]] string getTag() const noexcept override;

    public:
        [[nodiscard]] const ptr<const Children> children() const override;

    public:
        void render(cref<ReflowState> state_, const ptr<ReflowCommandBuffer> cmd_) override;

    public:
        math::vec2 prefetchDesiredSize(cref<ReflowState> state_, float scale_) const override;

        math::vec2 computeDesiredSize(cref<ReflowPassState> passState_) const override;

        void applyLayout(ref<ReflowState> state_, mref<LayoutContext> ctx_) override;

    public:
        bool willChangeLayout(cref<math::vec2> space_) const noexcept override;
    };
}
