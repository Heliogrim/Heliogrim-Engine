#pragma once

#include "../Input.hpp"
#include "../HBox.hpp"
#include "../Text.hpp"

namespace ember::engine::reflow {

    enum class InputFloatPrecision {
        eFloat,
        eDouble
    };

    class InputFloat :
        public Input<long double> {
    public:
        using this_type = InputFloat;

    public:
        InputFloat(mref<sptr<BoundStyleSheet>> style_, mref<sptr<BoundStyleSheet>> textStyle_);

        ~InputFloat() override;

    public:
        [[nodiscard]] string getTag() const noexcept override;

    private:
        sptr<HBox> _wrapper;
        sptr<Text> _text;

    private:
        void valueToText();

    private:
        string _value;

        math::vec2_t<input_type> _limits;
        input_type _stepSize;

        InputFloatPrecision _precision;

    public:
        void setValue(const input_type value_);

        void setLimits(const input_type lower_ = _STD numeric_limits<input_type>::min(),
            const input_type upper_ = _STD numeric_limits<input_type>::max());

        void setStepSize(const input_type stepSize_);

        void setPrecision(const InputFloatPrecision precision_);

    public:
        void updateValueAndValidity(const bool propagate_, const bool emit_) override;

    public:
        _STD function<void(input_type)> _callback = nullptr;

    public:
        [[nodiscard]] input_type value() const noexcept override;

    public:
        void render(const ptr<ReflowCommandBuffer> cmd_) override;

        void flow(cref<FlowContext> ctx_, cref<math::vec2> space_, cref<math::vec2> limit_, ref<StyleKeyStack> styleStack_) override;

        void shift(cref<FlowContext> ctx_, cref<math::vec2> offset_) override;

    public:
        [[nodiscard]] math::vec2 outerSize() const noexcept override;

        [[nodiscard]] math::vec2 innerSize() const noexcept override;

        [[nodiscard]] math::vec2 screenOffset() const noexcept override;

        [[nodiscard]] float shrinkFactor() const noexcept override;

        [[nodiscard]] float growFactor() const noexcept override;

    public:
        EventResponse onFocus(cref<FocusEvent> event_) override;

        EventResponse onBlur(cref<FocusEvent> event_) override;

        EventResponse onKeyDown(cref<KeyboardEvent> event_) override;

        EventResponse onKeyUp(cref<KeyboardEvent> event_) override;
    };

}
