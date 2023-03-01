#pragma once

#include "../Input.hpp"
#include "../HBox.hpp"
#include "../Text.hpp"

namespace hg::engine::reflow {
    class InputText :
        public Input<string> {
    public:
        using this_type = InputText;

    public:
        InputText(mref<sptr<BoundStyleSheet>> style_, mref<sptr<BoundStyleSheet>> textStyle_);

        ~InputText() override;

    public:
        [[nodiscard]] string getTag() const noexcept override;

    private:
        sptr<HBox> _wrapper;
        sptr<Text> _text;

    private:
        string _value;
        string _placeholder;

    public:
        void setPlaceholder(cref<string> placeholder_);

    public:
        [[nodiscard]] input_type value() const noexcept override;

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
        [[nodiscard]] math::vec2 outerSize() const noexcept override;

        [[nodiscard]] math::vec2 innerSize() const noexcept override;

        [[nodiscard]] math::vec2 screenOffset() const noexcept override;

    public:
        EventResponse onFocus(cref<FocusEvent> event_) override;

        EventResponse onBlur(cref<FocusEvent> event_) override;

        EventResponse onKeyDown(cref<KeyboardEvent> event_) override;

        EventResponse onKeyUp(cref<KeyboardEvent> event_) override;
    };
}
