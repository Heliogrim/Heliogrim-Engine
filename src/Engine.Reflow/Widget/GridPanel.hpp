#pragma once
#include "Panel.hpp"
#include "../Children.hpp"

namespace hg::engine::reflow {
    class GridPanel :
        public Panel {
    public:
        using this_type = GridPanel;

    public:
        GridPanel();

        ~GridPanel() override;

    protected:
        Children _children;

    public:
        const ptr<const Children> children() const override;

        void addChild(cref<sptr<Widget>> child_);

        void setChild(const u32 idx_, cref<sptr<Widget>> child_);

        void removeChild(cref<sptr<Widget>> child_);

        void clearChildren();
    };
}
