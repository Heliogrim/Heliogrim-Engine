#pragma once

#include "VBox.hpp"

namespace hg::engine::reflow {
    class Panel :
        public VBox {
    public:
        using this_type = Panel;

    public:
        Panel(mref<sptr<BoundStyleSheet>> style_);

        ~Panel() override;
    };
}
