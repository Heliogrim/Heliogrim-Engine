#pragma once

#include <concepts>
#include <map>
#include <ranges>
#include <Engine.Asserts/Asserts.hpp>
#include <Engine.Common/Forward.hpp>
#include <Engine.Common/Meta/Reference.hpp>

#include "Subscriber.hpp"
#include "Subscription.hpp"

namespace hg::editor::ui::rx {
	template <typename Type_>
	struct SubjectHolder {
		using type = std::conditional_t<std::is_reference_v<Type_>, std::add_pointer_t<::hg::meta::drop_ref_t<Type_>>, std::decay_t<Type_>>;
		type stored;
	};

	template <typename Type_>
	class Subject {
	public:
		using this_type = Subject<Type_>;
		using holder_type = SubjectHolder<Type_>;
		using subscriber_type = Subscriber<Type_>;
		using subscription_type = Subscription<Type_>;

	public:
		Subject(auto&&... args_) requires
			(not std::is_reference_v<Type_>) &&
			std::is_constructible_v<::hg::meta::drop_ref_t<Type_>, decltype(args_)&&...> :
			_holder(::hg::forward<decltype(args_)>(args_)...) {}

		Subject(auto& arg_) requires std::is_reference_v<Type_> && std::is_same_v<decltype(arg_), Type_> :
			_holder(std::addressof(arg_)) {}

		constexpr ~Subject() noexcept = default;

	private:
		holder_type _holder;
		std::map<Id, subscriber_type> _subscriptions;

	public:
		[[nodiscard]] static this_type of(auto&&... args_) noexcept requires
			std::is_constructible_v<this_type, decltype(args_)&&...> {
			return this_type { ::hg::forward<decltype(args_)>(args_)... };
		}

		[[nodiscard]] ref<const ::hg::meta::drop_ref_t<Type_>> peek() const noexcept {
			return _holder.stored;
		}

		void next(auto&&... args_) requires
			(not std::is_reference_v<Type_>) &&
			std::is_constructible_v<::hg::meta::drop_ref_t<Type_>, decltype(args_)&&...> {

			_holder.stored = typename holder_type::type { ::hg::forward<decltype(args_)>(args_)... };
			for (ref<const subscriber_type> subscriber : _subscriptions | std::views::values) {
				subscriber.invoke(_holder.stored);
			}
		}

		[[nodiscard]] subscription_type subscribe(auto&&... args_) requires
			std::is_constructible_v<subscriber_type, decltype(args_)&&...> {

			const auto nextId = _subscriptions.empty() ? Id { 0u } : Id { (--_subscriptions.end())->first.value + 1u };
			const auto result = _subscriptions.insert_or_assign(
				nextId,
				subscriber_type { ::hg::forward<decltype(args_)>(args_)... }
			);

			::hg::assertrt(result.second);
			return { .id = nextId, .subject = *this };
		}

		void unsubscribe(Id id_) {
			_subscriptions.erase(id_);
		}
	};

	template <typename Type_>
	Subject(Type_&&) -> Subject<::hg::meta::drop_ref_t<Type_>>;
}
