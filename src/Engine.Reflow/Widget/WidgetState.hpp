#pragma once

#include <Engine.Common/__macro.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::reflow {
	enum class InteractiveStateFlagBits : u8 {
		eFocus = 0b0000'0001,
		eHover = 0b0000'0010,
		eDisable = 0b0000'0100,
		ePress = 0b0000'1000,
		eDrag = 0b0001'0000,
		eSelected = 0b0010'0000
	};

	enum class WidgetStateFlagBits : u8 {
		eFocus = 0b0000'0001,
		eHover = 0b0000'0010,
		ePress = 0b0000'1000,
		/**/
		eVisible = 0b0001'0000,
		eSubtreePending = 0b0010'0000,
		eLayoutPending = 0b0100'0000,
		eRenderPending = 0b1000'0000
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

		[[nodiscard]] bool isPressed() const noexcept {
			return (*this) & WidgetStateFlagBits::ePress;
		}

		[[nodiscard]] bool isVisible() const noexcept {
			return (*this) & WidgetStateFlagBits::eVisible;
		}

		[[nodiscard]] constexpr bool isLayoutPending() const noexcept {
			return (*this) & WidgetStateFlagBits::eLayoutPending;
		}

		[[nodiscard]] constexpr bool isRenderPending() const noexcept {
			return (*this) & WidgetStateFlagBits::eRenderPending;
		}

		[[nodiscard]] constexpr bool isSubtreePending() const noexcept {
			return (*this) & WidgetStateFlagBits::eSubtreePending;
		}

		[[nodiscard]] constexpr bool isAnyPending() const noexcept {
			return isLayoutPending() || isRenderPending() || isSubtreePending();
		}

		[[nodiscard]] constexpr bool isLayoutOrRenderPending() const noexcept {
			return isLayoutPending() || isRenderPending();
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

		[[nodiscard]] constexpr u16 proxyRenderVersion() const noexcept {
			return _proxyRenderStateVersion;
		}

		constexpr void setProxyRenderVersion(const u16 version_) {
			_proxyRenderStateVersion = version_;
		}
	};
}
