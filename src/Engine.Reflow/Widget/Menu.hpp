#pragma once

#include "Popup.hpp"

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
        sptr<Widget> _content;

    public:
        [[nodiscard]] sptr<Widget> getContent() const noexcept;

        void setContent(cref<sptr<Widget>> content_);

    public:
        void render(const ptr<ReflowCommandBuffer> cmd_) override;

        void flow(
            cref<FlowContext> ctx_,
            cref<math::vec2> space_,
            cref<math::vec2> limit_,
            ref<StyleKeyStack> styleStack_
        ) override;

        void shift(cref<FlowContext> ctx_, cref<math::vec2> offset_) override;

    public:
        [[nodiscard]] const ptr<const Children> children() const override;

    public:
        [[nodiscard]] math::vec2 outerSize() const noexcept override;

        [[nodiscard]] math::vec2 innerSize() const noexcept override;

    public:
        EventResponse onFocus(cref<FocusEvent> event_) override;

        EventResponse onBlur(cref<FocusEvent> event_) override;
    };
}
