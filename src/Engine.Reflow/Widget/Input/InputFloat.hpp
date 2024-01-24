#pragma once

#include "../Input.hpp"
#include "../HorizontalPanel.hpp"
#include "../Text.hpp"
#include "../../Children.hpp"

namespace hg::engine::reflow {
    enum class InputFloatPrecision {
        eFloat,
        eDouble
    };

    class InputFloat :
        public Input<long double> {
    public:
        using this_type = InputFloat;

        using input_type = Input<long double>::input_type;

    public:
        InputFloat();

        ~InputFloat() override;

    public:
        [[nodiscard]] string getTag() const noexcept override;

    private:
    public:
        sptr<HorizontalPanel> _wrapper;
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

        void setLimits(
            const input_type lower_ = std::numeric_limits<input_type>::min(),
            const input_type upper_ = std::numeric_limits<input_type>::max()
        );

        void setStepSize(const input_type stepSize_);

        void setPrecision(const InputFloatPrecision precision_);

    public:
        void updateValueAndValidity(const bool propagate_, const bool emit_) override;

    public:
        std::function<void(input_type)> _callback = nullptr;

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
        [[nodiscard]] float shrinkFactor() const noexcept override;

        [[nodiscard]] float growFactor() const noexcept override;

    public:
        EventResponse onFocus(cref<FocusEvent> event_) override;

        EventResponse onBlur(cref<FocusEvent> event_) override;

        EventResponse onKeyDown(cref<KeyboardEvent> event_) override;

        EventResponse onKeyUp(cref<KeyboardEvent> event_) override;
    };
}
