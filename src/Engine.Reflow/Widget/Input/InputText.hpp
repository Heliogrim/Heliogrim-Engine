#pragma once

#include "../Input.hpp"
#include "../HorizontalPanel.hpp"
#include "../Text.hpp"
#include "../../Children.hpp"

namespace hg::engine::reflow {
    class InputText :
        public Input<string> {
    public:
        using this_type = InputText;

    public:
        InputText();

        ~InputText() override;

    public:
        [[nodiscard]] string getTag() const noexcept override;

    private:
    public:
        sptr<HorizontalPanel> _wrapper;
        sptr<Text> _text;

    private:
        string _value;
        string _placeholder;

    public:
        void setPlaceholder(cref<string> placeholder_);

    public:
        [[nodiscard]] input_type value() const noexcept override;

    private:
        SingleChildren _children;

    public:
        [[nodiscard]] const ptr<const SingleChildren> children() const override;

    public:
        void render(const ptr<ReflowCommandBuffer> cmd_) override;

    public:
        math::vec2 prefetchDesiredSize(cref<ReflowState> state_, float scale_) const override;

        math::vec2 computeDesiredSize(cref<ReflowPassState> passState_) const override;

        void applyLayout(ref<ReflowState> state_, mref<LayoutContext> ctx_) override;

    public:
        EventResponse onFocus(cref<FocusEvent> event_) override;

        EventResponse onBlur(cref<FocusEvent> event_) override;

        EventResponse onKeyDown(cref<KeyboardEvent> event_) override;

        EventResponse onKeyUp(cref<KeyboardEvent> event_) override;
    };
}
