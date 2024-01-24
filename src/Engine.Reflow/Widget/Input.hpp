#pragma once

#include "Widget.hpp"

namespace hg::engine::reflow {
    enum class InputControlStatus {
        eValid,
        eInvalid,
        ePending,
        eDisabled
    };

    class __declspec(novtable) InputBase : public Widget {
    protected:
        bool _resettable = false;

        /**
         * @see https://angular.io/api/forms/AbstractControl
         */
        InputControlStatus _status = InputControlStatus::eValid;

        bool _dirty = false;
        bool _touched = false;

    public:
        [[nodiscard]] bool isEnabled() const noexcept;

        [[nodiscard]] bool isDirty() const noexcept;

        [[nodiscard]] bool isTouched() const noexcept;

    public:
        virtual void enable();

        virtual void disable();

        virtual void markAsDirty();

        virtual void markAsPristine();

        virtual void markAsTouched();

        virtual void markAsUntouched();

        virtual void reset();

    public:
        virtual void updateValueAndValidity(const bool propagate_ = false, const bool emit_ = true);

    public:
        EventResponse onFocus(cref<FocusEvent> event_) override;

        EventResponse onBlur(cref<FocusEvent> event_) override;
    };

    template <typename InputType_>
    class Input :
        public InputBase {
    public:
        using this_type = Input<InputType_>;
        using input_type = std::decay_t<InputType_>;

    protected:
        Input() = default;

    public:
        ~Input() override = default;

    public:
        [[nodiscard]] virtual input_type value() const noexcept = 0;
    };
}
