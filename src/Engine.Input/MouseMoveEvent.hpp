#pragma once

#include <Engine.Common/Math/Vector.hpp>
#include <Engine.Event/Event.hpp>

namespace hg::engine::input::event {
	class MouseMoveEvent final :
		public StatelessEvent {
	public:
		using this_type = MouseMoveEvent;
		using reference_type = ref<this_type>;
		using const_reference_type = cref<this_type>;

	public:
		inline static constexpr event_type_id typeId { "MouseMoveEvent"_typeId };

	public:
		explicit MouseMoveEvent(
			ref<const math::fvec2> pointer_,
			ref<const math::fvec2> delta_,
			u32 button_,
			u32 modifier_
		) noexcept;

		MouseMoveEvent(cref<this_type> other_) noexcept = default;

		MouseMoveEvent(mref<this_type> other_) noexcept = default;

		~MouseMoveEvent() noexcept override = default;

		//private:
	public:
		math::fvec2 _pointer;
		math::fvec2 _delta;

		u32 _button;
		u32 _modifier;
	};
}