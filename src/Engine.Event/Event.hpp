#pragma once

#include <concepts>

#include "EventTypeId.hpp"

namespace hg {
	class macro_novtable Event {
	public:
		/**
		 * Problem: Artifact due to stateful events which are handled like a stateless call-graph
		 */
		constexpr Event() noexcept = default;

		constexpr Event(Event&&) noexcept = default;

		constexpr Event(const Event&) noexcept = default;

		constexpr virtual ~Event() noexcept = default;

	public:
		constexpr Event& operator=(Event&&) noexcept = default;

		constexpr Event& operator=(const Event&) noexcept = default;
	};

	class macro_novtable StatelessEvent : public Event {};

	class macro_novtable StatefulEvent : public Event {
	public:
		constexpr ~StatefulEvent() noexcept override = default;

	public:
		/**
		 * Check if this event was canceled
		 *
		 * @author Julius
		 * @date 07.10.2021
		 *
		 * @returns True if it succeeds, false if it fails.
		 */
		[[nodiscard]] virtual bool canceled() const noexcept = 0;

		/**
		 * Cancel this event
		 *
		 * @author Julius
		 * @date 07.10.2021
		 */
		virtual void cancel() noexcept = 0;
	};

	template <class EventType_>
	concept IsEvent = hg::HasStaticType<EventType_, event_type_id>;

	template <class EventType_>
	concept IsStatefulEvent = IsEvent<EventType_> && requires(const EventType_ obj) {
			{ obj.canceled() } -> std::same_as<bool>;
		} && requires(EventType_ obj) {
			{ obj.cancel() } -> std::same_as<void>;
		};

	template <class EventType_>
	concept IsStatelessEvent = IsEvent<EventType_> && !IsStatefulEvent<EventType_>;
}
