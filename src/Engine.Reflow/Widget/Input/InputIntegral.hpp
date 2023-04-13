#pragma once

#include "../Input.hpp"
#include "../HorizontalPanel.hpp"
#include "../Text.hpp"

namespace hg::engine::reflow {
    class InputIntegral :
        public Input<s64> {
    public:
        using this_type = InputIntegral;

    public:
        InputIntegral();

        ~InputIntegral() override;

    public:
        [[nodiscard]] string getTag() const noexcept override;

    private:
    public:
        sptr<HorizontalPanel> _wrapper;
        sptr<Text> _text;

    private:
        string _value;

        math::vec2_t<s64> _limits;
        s64 _stepSize;

    public:
        void setValue(const s64 value_);

        void setLimits(
            const s64 lower_ = _STD numeric_limits<s64>::min(),
            const s64 upper_ = _STD numeric_limits<s64>::max()
        );

        void setStepSize(const s64 stepSize_);

    public:
        void updateValueAndValidity(const bool propagate_, const bool emit_) override;

    public:
        _STD function<void(input_type)> _callback = nullptr;

    public:
        [[nodiscard]] input_type value() const noexcept override;

    private:
        SingleChildren _children;

    public:
        [[nodiscard]] const ptr<const SingleChildren> children() const override;

    public:
        void render(cref<ReflowState> state_, const ptr<ReflowCommandBuffer> cmd_) override;

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
