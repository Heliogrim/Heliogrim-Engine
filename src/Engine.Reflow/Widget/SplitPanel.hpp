#pragma once
#include "Panel.hpp"
#include "../Children.hpp"
#include "../Padding.hpp"
#include "../ReflowAlignment.hpp"
#include "../ReflowOrientation.hpp"
#include "../ReflowSpacing.hpp"
#include "../ReflowUnit.hpp"
#include "../Attribute/StyleAttribute.hpp"
#include "../Style/SplitStyle.hpp"

namespace hg::engine::reflow {
    struct SplitSlot {
        float proportion;
    };

    class SplitPanel :
        public Panel {
    public:
        using this_type = SplitPanel;

    public:
        SplitPanel();

        ~SplitPanel() override;

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

            Attribute<ReflowOrientation> orientation;

            Attribute<float> flexGrow;
            Attribute<float> flexShrink;

            struct SlotAttributes {
                Attribute<ReflowSpacing> justify;
                Attribute<ReflowAlignment> align;

                Attribute<Padding> padding;
            } slot;

            StyleAttribute<SplitStyle> style;
        } attr;

    protected:
        Children _children;
        Vector<SplitSlot> _slots;

    public:
        const ptr<const Children> children() const override;

        void addChild(cref<sptr<Widget>> child_);

        void setChild(const u32 idx_, cref<sptr<Widget>> child_);

        void removeChild(cref<sptr<Widget>> child_);

        void clearChildren();

    public:
        math::vec2 prefetchDesiredSize(cref<ReflowState> state_, float scale_) const override;

        math::vec2 computeDesiredSize(cref<ReflowPassState> passState_) const override;

        void applyLayout(ref<ReflowState> state_, mref<LayoutContext> ctx_) override;

    private:
        void recomputeSlotPortions(float unnormalized = 0.F);
    };
}
