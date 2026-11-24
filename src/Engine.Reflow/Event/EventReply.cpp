#include "EventReply.hpp"

using namespace hg::engine::reflow;
using namespace hg;

EventReply EventReply::unhandled() noexcept {
	return EventReply { EventResponse::eUnhandled };
}

EventReply EventReply::handled() noexcept {
	return EventReply { EventResponse::eHandled };
}

EventReply EventReply::consumed() noexcept {
	return EventReply { EventResponse::eConsumed };
}

EventReply EventReply::withCapture() && noexcept {
	capturePointer = true;
	return std::move(*this);
}

EventReply EventReply::withRelease() && noexcept {
	releaseCapture = true;
	return std::move(*this);
}

EventReply EventReply::withFocus(wptr<Widget> target_) && noexcept {
	focus = std::move(target_);
	return std::move(*this);
}

bool EventReply::isConsumed() const noexcept {
	return response == EventResponse::eConsumed;
}

EventReply hg::engine::reflow::merge(EventReply lhs_, ref<const EventReply> rhs_) noexcept {
	lhs_.response = std::max(lhs_.response, rhs_.response);
	lhs_.capturePointer = lhs_.capturePointer || rhs_.capturePointer;
	lhs_.releaseCapture = lhs_.releaseCapture || rhs_.releaseCapture;
	if (not rhs_.focus.expired()) {
		lhs_.focus = rhs_.focus;
	}
	return lhs_;
}
