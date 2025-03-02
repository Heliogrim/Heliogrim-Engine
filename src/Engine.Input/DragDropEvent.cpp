#include "DragDropEvent.hpp"

#include <Engine.Common/Move.hpp>

using namespace hg::engine::input::event;
using namespace hg;

DragDropEvent::DragDropEvent(
	cref<math::ivec2> pointer_,
	mref<DragDropEventPayload> payload_
) noexcept :
	StatelessEvent(),
	_pointer(pointer_),
	_payload(::hg::move(payload_)) {}

DragDropEvent::DragDropEvent(cref<this_type> other_) noexcept :
	StatelessEvent(),
	_pointer(other_._pointer),
	_payload(other_._payload) {}

DragDropEvent::DragDropEvent(mref<this_type> other_) noexcept :
	StatelessEvent(),
	_pointer(other_._pointer),
	_payload(::hg::move(other_._payload)) {}

DragDropEvent::~DragDropEvent() noexcept {}
