#pragma once
#include "Panel.hpp"
#include "../Children.hpp"
#include "../ReflowUnit.hpp"
#include "../Padding.hpp"
#include "../ReflowSpacing.hpp"
#include "../ReflowAlignment.hpp"
#include "../Attribute/StyleAttribute.hpp"
#include "../Style/PanelStyle.hpp"

namespace hg::engine::reflow {
    class VerticalPanel :
        public Panel {
    public:
        using this_type = VerticalPanel;

    public:
        VerticalPanel();

        ~VerticalPanel() override;

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

            Attribute<Padding> padding;

            Attribute<ReflowSpacing> justify;
            Attribute<ReflowAlignment> align;

            Attribute<float> colGap;
            Attribute<float> rowGap;

            Attribute<float> flexGrow;
            Attribute<float> flexShrink;

            StyleAttribute<PanelStyle> style;
        } attr;

    protected:
        Children _children;

    public:
        const ptr<const Children> children() const override;

        void addChild(cref<sptr<Widget>> child_);

        void setChild(const u32 idx_, cref<sptr<Widget>> child_);

        void removeChild(cref<sptr<Widget>> child_);

        void clearChildren();

    public:
        void render(const ptr<ReflowCommandBuffer> cmd_) override;

    public:
        math::vec2 prefetchDesiredSize(cref<ReflowState> state_, float scale_) const override;

        math::vec2 computeDesiredSize(cref<ReflowPassState> passState_) const override;

        void applyLayout(ref<ReflowState> state_, mref<LayoutContext> ctx_) override;

    public:
        [[nodiscard]] float shrinkFactor() const noexcept override;

        [[nodiscard]] float growFactor() const noexcept override;
    };
}
