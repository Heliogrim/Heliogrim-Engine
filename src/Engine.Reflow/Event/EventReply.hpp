#pragma once

#include "EventResponse.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::reflow {
	class Widget;
}

namespace hg::engine::reflow {
	struct EventReply {
	public:
		constexpr EventReply() noexcept = default;

		constexpr EventReply(const EventResponse response_) noexcept :
			response(response_) {}

	public:
		EventResponse response = EventResponse::eUnhandled;
		bool capturePointer = false;
		bool releaseCapture = false;
		wptr<Widget> focus {};

	public:
		[[nodiscard]] static EventReply unhandled() noexcept;

		[[nodiscard]] static EventReply handled() noexcept;

		[[nodiscard]] static EventReply consumed() noexcept;

		[[nodiscard]] EventReply withCapture() && noexcept;

		[[nodiscard]] EventReply withRelease() && noexcept;

		[[nodiscard]] EventReply withFocus(wptr<Widget> target_) && noexcept;

		[[nodiscard]] bool isConsumed() const noexcept;
	};

	[[nodiscard]] EventReply merge(EventReply lhs_, ref<const EventReply> rhs_) noexcept;
}
