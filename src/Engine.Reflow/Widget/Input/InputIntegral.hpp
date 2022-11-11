#pragma once

#include "../Input.hpp"
#include "../HBox.hpp"
#include "../Text.hpp"

namespace ember::engine::reflow {

    class InputIntegral :
        public Input<s64> {
    public:
        using this_type = InputIntegral;

    public:
        InputIntegral(mref<sptr<BoundStyleSheet>> style_, mref<sptr<BoundStyleSheet>> textStyle_);

        ~InputIntegral() override;

    private:
        sptr<HBox> _wrapper;
        sptr<Text> _text;

    private:
        string _value;

        math::vec2_t<s64> _limits;
        s64 _stepSize;

    public:
        void setValue(const s64 value_);

        void setLimits(const s64 lower_ = _STD numeric_limits<s64>::min(),
            const s64 upper_ = _STD numeric_limits<s64>::max());

        void setStepSize(const s64 stepSize_);

    public:
        void updateValueAndValidity(const bool propagate_, const bool emit_) override;

    public:
        [[nodiscard]] input_type value() const noexcept override;

    public:
        void render(const ptr<ReflowCommandBuffer> cmd_) override;

        void flow(cref<FlowContext> ctx_, cref<math::vec2> space_, ref<StyleKeyStack> styleStack_) override;

        void shift(cref<FlowContext> ctx_, cref<math::vec2> offset_) override;

    public:
        [[nodiscard]] math::vec2 outerSize() const noexcept override;

        [[nodiscard]] math::vec2 innerSize() const noexcept override;

        [[nodiscard]] math::vec2 screenOffset() const noexcept override;

    public:
        EventResponse onKeyDown(cref<KeyboardEvent> event_) override;

        EventResponse onKeyUp(cref<KeyboardEvent> event_) override;
    };

}
