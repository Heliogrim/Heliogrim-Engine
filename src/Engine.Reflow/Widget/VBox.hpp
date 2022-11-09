#pragma once
#include "Box.hpp"

namespace ember::engine::reflow {

    class VBox :
        public Box {
    public:
        using this_type = VBox;

    public:
        VBox(mref<sptr<BoundStyleSheet>> style_);

        ~VBox() override;

    public:
        void flow(cref<FlowContext> ctx_, cref<math::vec2> space_, ref<StyleKeyStack> styleStack_) override;

        void shift(cref<FlowContext> ctx_, cref<math::vec2> offset_) override;
    };

}
