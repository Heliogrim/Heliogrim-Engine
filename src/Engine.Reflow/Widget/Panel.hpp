#pragma once

#include "Widget.hpp"

namespace hg::engine::reflow {
    struct PanelStyle;
}

namespace hg::engine::reflow {
    class Panel :
        public Widget {
    public:
        using this_type = Panel;

    public:
        Panel();

        ~Panel() override;

    protected:
        void renderPanel(const ptr<ReflowCommandBuffer> cmd_, cref<PanelStyle> style_) const;

    public:
        void render(const ptr<ReflowCommandBuffer> cmd_) override;
    };
}
