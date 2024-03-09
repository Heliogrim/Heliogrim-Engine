#pragma once

#include <Engine.Common/Flag.hpp>
#include <Engine.Common/Math/Vector.hpp>

namespace hg::engine::reflow {
    enum class WidgetStateFlagBits : u8 {
        eFocus = 0b0000'0001,
        eHover = 0b0000'0010,
        eVisible = 0b0000'0100,
        /**/
        ePending = 0b0001'0000,
        ePendingInherit = 0b0010'0000
    };

    typedef Flag<WidgetStateFlagBits, hg::u8> WidgetStateFlag;

    struct WidgetState :
        public WidgetStateFlag {
    public:
        constexpr WidgetState(const WidgetStateFlag initialFlags_) noexcept :
            Flag(initialFlags_) {}

    public:
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

        [[nodiscard]] bool isProxyPending() const noexcept {
            return unwrap & (static_cast<value_type>(WidgetStateFlagBits::ePending) |
                static_cast<value_type>(WidgetStateFlagBits::ePendingInherit));
        }

    public:
        void set(const WidgetStateFlagBits flag_) {
            (*this) |= flag_;
        }

        void unset(const WidgetStateFlagBits flag_) {
            this->unwrap &= (~static_cast<WidgetState::value_type>(flag_));
        }

    private:
        u16 _proxyRenderStateVersion = 0u;
        u16 _renderStateVersion = 0u;

    public:
        [[nodiscard]] constexpr bool shouldRender(const u16 compareVersion_) const noexcept {
            return _renderStateVersion >= compareVersion_ || _proxyRenderStateVersion >= compareVersion_;
        }

        [[nodiscard]] constexpr bool shouldRenderSelf(const u16 compareVersion_) const noexcept {
            return _renderStateVersion >= compareVersion_;
        }

        [[nodiscard]] constexpr bool shouldProxyRender(const u16 compareVersion_) const noexcept {
            return _proxyRenderStateVersion >= compareVersion_;
        }

        constexpr void setRenderVersion(const u16 version_) {
            _renderStateVersion = version_;
        }

        constexpr void setProxyRenderVersion(const u16 version_) {
            _proxyRenderStateVersion = version_;
        }
    };
}
