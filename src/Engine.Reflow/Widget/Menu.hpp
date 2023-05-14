#pragma once

#include "Popup.hpp"
#include "../Children.hpp"

namespace hg::engine::reflow {
    class Menu :
        public Popup {
    public:
        using this_type = Menu;

    public:
        Menu();

        ~Menu() override;

    public:
        [[nodiscard]] string getTag() const noexcept override;

    public:
        void openMenu();

        void closeMenu();

    private:
        SingleChildren _children;

    public:
        [[nodiscard]] const ptr<const SingleChildren> children() const override;

    public:
        [[nodiscard]] sptr<Widget> getContent() const noexcept;

        void setContent(cref<sptr<Widget>> content_);

    public:
        void render(const ptr<ReflowCommandBuffer> cmd_) override;

    public:
        math::vec2 prefetchDesiredSize(cref<ReflowState> state_, float scale_) const override;

        math::vec2 computeDesiredSize(cref<ReflowPassState> passState_) const override;

        void applyLayout(ref<ReflowState> state_, mref<LayoutContext> ctx_) override;

    public:
        EventResponse onFocus(cref<FocusEvent> event_) override;

        EventResponse onBlur(cref<FocusEvent> event_) override;
    };
}
