#pragma once

#include "VBox.hpp"

namespace ember::engine::reflow {

    class Panel :
        public VBox {
    public:
        using this_type = Panel;

    public:
        Panel(mref<sptr<BoundStyleSheet>> style_);

        ~Panel() override;
    };

}
