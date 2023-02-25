#pragma once
#include "Box.hpp"

namespace ember::engine::reflow {
    class HBox :
        public Box {
    public:
        using this_type = HBox;

    public:
        HBox(mref<sptr<BoundStyleSheet>> style_);

        ~HBox() override;

    public:
        [[nodiscard]] string getTag() const noexcept override;

    public:
        void flow(
            cref<FlowContext> ctx_,
            cref<math::vec2> space_,
            cref<math::vec2> limit_,
            ref<StyleKeyStack> styleStack_
        ) override;

        void shift(cref<FlowContext> ctx_, cref<math::vec2> offset_) override;
    };
}
