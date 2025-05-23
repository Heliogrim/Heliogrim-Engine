#pragma once

#include <Engine.Event/Event.hpp>

namespace hg::engine::reflow {
	class FormSubmitEvent :
		public StatelessEvent {
	public:
		using this_type = FormSubmitEvent;
		using reference_type = ref<this_type>;
		using const_reference_type = cref<this_type>;

	public:
		inline static constexpr event_type_id typeId { "FormSubmitEvent"_typeId };

	public:
		constexpr explicit FormSubmitEvent() noexcept = default;

		FormSubmitEvent(cref<this_type> other_) noexcept = default;

		FormSubmitEvent(mref<this_type> other_) noexcept = default;

		constexpr ~FormSubmitEvent() noexcept override = default;
	};
}
