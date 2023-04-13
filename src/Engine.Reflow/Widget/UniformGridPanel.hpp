#pragma once

#include "GridPanel.hpp"
#include "../Algorithm/FlexBox.hpp"
#include "../ReflowUnit.hpp"
#include "../Padding.hpp"

namespace hg::engine::reflow {
    class UniformGridPanel :
        public GridPanel {
    public:
        using this_type = UniformGridPanel;

    public:
        UniformGridPanel();

        ~UniformGridPanel() override;

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

            Attribute<algorithm::FlexLineOrientation> orientation;
            Attribute<ReflowSpacing> justify;
            Attribute<ReflowAlignment> align;

            Attribute<float> colGap;
            Attribute<float> rowGap;

            struct SlotAttributes {
                Attribute<ReflowUnit> width;
                Attribute<ReflowUnit> height;

                Attribute<Padding> padding;

                Attribute<ReflowAlignment> justify;
                Attribute<ReflowAlignment> align;
            } slot;
        } attr;

    public:
        [[nodiscard]] math::vec2 computeGridItemSize() const;

    public:
        math::vec2 prefetchDesiredSize(cref<ReflowState> state_, float scale_) const override;

        math::vec2 computeDesiredSize(cref<ReflowPassState> passState_) const override;

        void applyLayout(ref<ReflowState> state_, mref<LayoutContext> ctx_) override;
    };
}
