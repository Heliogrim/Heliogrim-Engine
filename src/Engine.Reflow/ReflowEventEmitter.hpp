#pragma once

#include <functional>
#include <map>
#include <ranges>
#include <Engine.Common/Move.hpp>
#include <Engine.Common/Types.hpp>
#include <Engine.Event/Event.hpp>

#include "Event/EventResponse.hpp"

namespace hg::engine::reflow {
	class ReflowEventEmitter final {
	public:
		using this_type = ReflowEventEmitter;

		using handle_type = u8;
		using handle_listener_map = std::map<u8, std::function<EventResponse(ref<const StatelessEvent>)>>;

		template <std::derived_from<StatelessEvent> EventType_>
		using listener_type = std::function<EventResponse(ref<const EventType_>)>;

	public:
		constexpr ReflowEventEmitter() noexcept = default;

		constexpr ~ReflowEventEmitter() noexcept = default;

	private:
		u8 _handleCount;
		std::map<StringView, handle_listener_map> _listener;

	public:
		template <std::derived_from<StatelessEvent> EventType_>
		[[nodiscard]] handle_type on(StringView eventName_, mref<std::function<EventResponse(ref<const EventType_>)>> listenFn_) {

			const auto handle = ++_handleCount;

			if (not _listener.contains(eventName_)) {
				_listener.insert_or_assign(eventName_, handle_listener_map {});
			}

			// Error: Unsafe cast with type erasure
			_listener.at(eventName_).insert_or_assign(
				handle,
				reinterpret_cast<std::function<EventResponse(ref<const StatelessEvent>)>&&>(::hg::move(listenFn_))
			);
			return handle;
		}

		template <std::derived_from<StatelessEvent> EventType_>
		[[nodiscard]] bool off(StringView eventName_, const handle_type handle_) noexcept {

			if (not _listener.contains(eventName_)) {
				return false;
			}

			return _listener.at(eventName_).erase(handle_) > 0;
		}

		template <std::derived_from<StatelessEvent> EventType_>
		[[nodiscard]] EventResponse emit(StringView eventName_, ref<const EventType_> event_) const {

			const auto where = _listener.find(eventName_);
			if (where == _listener.end()) {
				return EventResponse::eUnhandled;
			}

			/**/

			auto response = EventResponse::eUnhandled;
			for (const auto& listen : where->second | std::views::values) {
				const auto result = listen(event_);
				response = std::max(response, result);

				if (response == EventResponse::eConsumed) {
					break;
				}
			}

			return response;
		}
	};
}
