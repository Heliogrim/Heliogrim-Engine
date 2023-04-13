#pragma once

#include "Widget.hpp"
#include "../Window/__fwd.hpp"

namespace hg::engine::reflow {
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

    public:
        math::vec2 prefetchDesiredSize(cref<ReflowState> state_, float scale_) const override;

        void applyLayout(ref<ReflowState> state_, mref<LayoutContext> ctx_) override;
    };
}
