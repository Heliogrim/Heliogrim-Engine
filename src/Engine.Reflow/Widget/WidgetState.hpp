#pragma once

#include <Engine.Common/Flag.hpp>

namespace hg::engine::reflow {
    enum class WidgetStateFlagBits : u8 {
        eFocus = 0b0000'0001,
        eHover = 0b0000'0010,
        eVisible = 0b0000'0100,
        /**/
        ePending = 0b0000'1000,
        ePendingInherit = 0b0001'0000,
        eShift = 0b0010'0000,
        eShiftInherit = 0b0100'0000,
        eCapture = 0b1000'0000
    };

    typedef Flag<WidgetStateFlagBits, hg::u8> WidgetStateFlag;

    struct WidgetState :
        public WidgetStateFlag {
        [[nodiscard]] bool isFocus() const noexcept {
            return (*this) & WidgetStateFlagBits::eFocus;
        }

        [[nodiscard]] bool isHover() const noexcept {
            return (*this) & WidgetStateFlagBits::eHover;
        }

        [[nodiscard]] bool isVisible() const noexcept {
            return (*this) & WidgetStateFlagBits::eVisible;
        }

        [[nodiscard]] bool isPending() const noexcept {
            return (*this) & WidgetStateFlagBits::ePending;
        }

        [[nodiscard]] bool isPendingInherit() const noexcept {
            return (*this) & WidgetStateFlagBits::ePendingInherit;
        }

        [[nodiscard]] bool isShift() const noexcept {
            return (*this) & WidgetStateFlagBits::eShift;
        }

        [[nodiscard]] bool isShiftInherit() const noexcept {
            return (*this) & WidgetStateFlagBits::eShiftInherit;
        }

        [[nodiscard]] bool isCapture() const noexcept {
            return (*this) & WidgetStateFlagBits::eCapture;
        }

        [[nodiscard]] bool isProxyPending() const noexcept {
            return unwrap & (static_cast<value_type>(WidgetStateFlagBits::ePending) |
                static_cast<value_type>(WidgetStateFlagBits::ePendingInherit));
        }

        [[nodiscard]] bool isProxyShift() const noexcept {
            return unwrap & (static_cast<value_type>(WidgetStateFlagBits::eShift) |
                static_cast<value_type>(WidgetStateFlagBits::eShiftInherit));
        }

    public:
        void set(const WidgetStateFlagBits flag_) {
            (*this) |= flag_;
        }

        void unset(const WidgetStateFlagBits flag_) {
            this->unwrap &= (~static_cast<WidgetState::value_type>(flag_));
        }
    };
}
