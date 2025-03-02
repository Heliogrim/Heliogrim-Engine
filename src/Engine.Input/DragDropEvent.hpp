#pragma once

#include <Engine.Common/String.hpp>
#include <Engine.Common/Variant.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/Math/Vector.hpp>
#include <Engine.Event/Event.hpp>

namespace hg::engine::input::event {
	struct DragDropEventFilePayload final {
		Vector<string> paths;
	};

	struct DragDropEventTextPayload final {
		string data;
	};

	using DragDropEventPayload = Variant<DragDropEventFilePayload, DragDropEventTextPayload>;

	class DragDropEvent final :
		public StatelessEvent {
	public:
		using this_type = DragDropEvent;
		using reference_type = ref<this_type>;
		using const_reference_type = cref<this_type>;

	public:
		inline static constexpr event_type_id typeId { "DragDropEvent"_typeId };

	public:
		explicit DragDropEvent(
			cref<math::ivec2> pointer_,
			mref<DragDropEventPayload> payload_
		) noexcept;

		DragDropEvent(cref<this_type> other_) noexcept;

		DragDropEvent(mref<this_type> other_) noexcept;

		~DragDropEvent() noexcept override;

		//private:
	public:
		math::ivec2 _pointer;
		DragDropEventPayload _payload;
	};
}
