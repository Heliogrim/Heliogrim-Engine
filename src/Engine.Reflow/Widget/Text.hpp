#pragma once
#include "Widget.hpp"
#include "../Style/__fwd.hpp"
#include "../Style/StyleSheet.hpp"

namespace hg::engine::reflow {
    class Text :
        public Widget {
    public:
        using this_type = Text;

    public:
        Text(mref<sptr<BoundStyleSheet>> style_);

        ~Text() override;

    public:
        [[nodiscard]] string getTag() const noexcept override;

    private:
        sptr<BoundStyleSheet> _style;
        StyleSheet _computedStyle;
        string _text;

    public:
        [[nodiscard]] ref<BoundStyleSheet> style() noexcept;

    public:
        [[nodiscard]] cref<string> getText() const noexcept;

        void setText(cref<string> text_);

    private:
        math::vec2 contentSize(cref<math::vec2> space_) const;

    public:
        void render(const ptr<ReflowCommandBuffer> cmd_) override;

        void flow(
            cref<FlowContext> ctx_,
            cref<math::vec2> space_,
            cref<math::vec2> limit_,
            ref<StyleKeyStack> styleStack_
        ) override;

        void shift(cref<FlowContext> ctx_, cref<math::vec2> offset_) override;

    private:
        math::vec2 _contentSize;
        math::vec2 _innerSize;
        math::vec2 _screenOff;

    public:
        [[nodiscard]] math::vec2 outerSize() const noexcept override;

        [[nodiscard]] math::vec2 innerSize() const noexcept override;

        [[nodiscard]] math::vec2 screenOffset() const noexcept override;

    private:
        StyleKeyStack::set_type _prevStyle;
        math::vec2 _prevSpace;

    public:
        [[nodiscard]] bool willChangeLayout(
            cref<math::vec2> space_,
            cref<StyleKeyStack> styleStack_
        ) const noexcept override;
    };
}
