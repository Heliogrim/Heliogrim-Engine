#pragma once
#include "Widget.hpp"

namespace ember::engine::gfx::glow::ui {

    class Popup : public Widget {
    public:
        using this_type = Popup;

    public:
        Popup();

        ~Popup() override = default;

    public:
        math::vec2 flow(cref<UIContext> ctx_, cref<math::vec2> available_) override;

        void shift(cref<UIContext> ctx_, cref<math::vec2> offset_) override;

    protected:
        math::vec2 _anchor;

    public:
        [[nodiscard]] cref<math::vec2> anchor() const noexcept;

        void setAnchor(cref<math::vec2> anchor_);

    protected:
        wptr<Widget> _root;

    public:
        [[nodiscard]] cref<wptr<Widget>> root() const noexcept;

        void setRoot(cref<wptr<Widget>> root_);
    };

}
