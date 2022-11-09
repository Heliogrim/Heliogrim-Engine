#pragma once

#include "Popup.hpp"
#include "../Style/__fwd.hpp"
#include "../Style/StyleSheet.hpp"
#include "../Children.hpp"

namespace ember::engine::reflow {

    class Dialog :
        public Popup {
    public:
        using this_type = Dialog;

    public:
        Dialog(mref<sptr<BoundStyleSheet>> style_);

        ~Dialog() override;

    private:
        bool _resizable;
        bool _closeable;

        string _title;

    private:
        sptr<Widget> _titleBar;
        sptr<Widget> _content;

        Children _children;

        sptr<BoundStyleSheet> _style;
        StyleSheet _computedStyle;

    private:
        void repackChildren();

    public:
        [[nodiscard]] sptr<Widget> getTitleBar() const noexcept;

        [[nodiscard]] sptr<Widget> getContent() const noexcept;

        void setContent(cref<sptr<Widget>> content_);

    public:
        void render(const ptr<ReflowCommandBuffer> cmd_) override;

        void flow(cref<FlowContext> ctx_, cref<math::vec2> space_, ref<StyleKeyStack> styleStack_) override;

        void shift(cref<FlowContext> ctx_, cref<math::vec2> offset_) override;

    public:
        [[nodiscard]] const ptr<const Children> children() const override;

    private:
        math::vec2 _compositeSize;

    public:
        [[nodiscard]] math::vec2 outerSize() const noexcept override;

        [[nodiscard]] math::vec2 innerSize() const noexcept override;
    };

}
