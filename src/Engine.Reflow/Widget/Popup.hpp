#pragma once

#include "Widget.hpp"
#include "../Window/__fwd.hpp"

namespace ember::engine::reflow {
    class Popup :
        public Widget {
    public:
        using this_type = Popup;

    protected:
        Popup();

    public:
        ~Popup() override;

    private:
        non_owning_rptr<PopupLayer> _layer;

    public:
        void setPopupLayer(const non_owning_rptr<PopupLayer> layer_);

        [[nodiscard]] non_owning_rptr<PopupLayer> getPopupLayer() const noexcept;

    public:
        [[nodiscard]] sptr<Widget> root() const override;

        [[nodiscard]] ReflowPosition position() const noexcept override;

        [[nodiscard]] math::vec2 screenOffset() const noexcept override;
    };
}
