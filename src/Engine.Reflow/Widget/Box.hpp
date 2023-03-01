#pragma once

#include "Widget.hpp"
#include "../Style/__fwd.hpp"
#include "../Style/StyleSheet.hpp"

#include "../Children.hpp"

namespace hg::engine::reflow {
    class Box :
        public Widget {
    public:
        using this_type = Box;

    protected:
        Box(mref<sptr<BoundStyleSheet>> style_);

    public:
        ~Box() override;

    protected:
        sptr<BoundStyleSheet> _style;
        StyleSheet _computedStyle;

    public:
        [[nodiscard]] cref<sptr<BoundStyleSheet>> style() const noexcept;

        [[nodiscard]] ref<sptr<BoundStyleSheet>> style() noexcept;

    protected:
        Children _children;

    public:
        [[nodiscard]] const ptr<const Children> children() const override;

        void addChild(cref<sptr<Widget>> child_);

        void removeChild(cref<sptr<Widget>> child_);

        void clearChildren();

    protected:
        math::vec2 _innerSize;
        math::vec2 _screenOff;

    public:
        void render(const ptr<ReflowCommandBuffer> cmd_) override;

    public:
        [[nodiscard]] float shrinkFactor() const noexcept override;

        [[nodiscard]] float growFactor() const noexcept override;

        [[nodiscard]] math::vec2 outerSize() const noexcept override;

        [[nodiscard]] math::vec2 innerSize() const noexcept override;

        [[nodiscard]] math::vec2 screenOffset() const noexcept override;

    protected:
        StyleKeyStack::set_type _prevStyleStack;
        math::vec2 _prevSpace;

    public:
        [[nodiscard]] bool willChangeLayout(
            cref<math::vec2> space_,
            cref<StyleKeyStack> styleStack_
        ) const noexcept override;
    };
}
