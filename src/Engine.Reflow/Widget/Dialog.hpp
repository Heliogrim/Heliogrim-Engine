#pragma once

#include "Popup.hpp"
#include "../Children.hpp"
#include "../ReflowUnit.hpp"
#include "../Padding.hpp"

namespace hg::engine::reflow {
    class Dialog :
        public Popup {
    public:
        using this_type = Dialog;

    public:
        Dialog();

        ~Dialog() override;

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

            Attribute<engine::color> color;
        } attr;

    private:
        bool _resizable;
        bool _closeable;

        string _title;

    private:
        FixedChildren<2> _children;

    public:
        [[nodiscard]] const ptr<const Children> children() const override;

    public:
        [[nodiscard]] sptr<Widget> getTitleBar() const noexcept;

        void setTitleBar(cref<sptr<Widget>> titleBar_);

        [[nodiscard]] sptr<Widget> getContent() const noexcept;

        void setContent(cref<sptr<Widget>> content_);

    public:
        void render(cref<ReflowState> state_, const ptr<ReflowCommandBuffer> cmd_) override;

    public:
        math::vec2 prefetchDesiredSize(cref<ReflowState> state_, float scale_) const override;

        math::vec2 computeDesiredSize(cref<ReflowPassState> passState_) const override;

        void applyLayout(ref<ReflowState> state_, mref<LayoutContext> ctx_) override;
    };
}
