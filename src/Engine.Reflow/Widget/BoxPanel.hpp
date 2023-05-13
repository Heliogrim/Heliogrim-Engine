#pragma once

#include "Panel.hpp"
#include "../Children.hpp"
#include "../ReflowUnit.hpp"
#include "../Padding.hpp"
#include "../ReflowSpacing.hpp"
#include "../ReflowAlignment.hpp"

namespace hg::engine::reflow {
    class BoxPanel :
        public Panel {
    public:
        using this_type = BoxPanel;

    public:
        BoxPanel();

        ~BoxPanel() override;

    public:
        string getTag() const noexcept override;

    protected:
        struct Attributes {
            Attribute<ReflowUnit> minWidth;
            Attribute<ReflowUnit> width;
            Attribute<ReflowUnit> maxWidth;

            Attribute<ReflowUnit> minHeight;
            Attribute<ReflowUnit> height;
            Attribute<ReflowUnit> maxHeight;

            Attribute<Padding> padding;

            Attribute<ReflowSpacing> justify;
            Attribute<ReflowAlignment> align;
        } _attr;

    public:
        [[nodiscard]] virtual ref<Attributes> attributes() noexcept;

    protected:
        SingleChildren _children;

    public:
        [[nodiscard]] const ptr<const Children> children() const override;

        void setChild(cref<sptr<Widget>> nextChild_);

    public:
        math::vec2 prefetchDesiredSize(cref<ReflowState> state_, float scale_) const override;

        math::vec2 computeDesiredSize(cref<ReflowPassState> passState_) const override;

        void applyLayout(ref<ReflowState> state_, mref<LayoutContext> ctx_) override;
    };
}
