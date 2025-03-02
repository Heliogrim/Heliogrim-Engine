#pragma once

#include <Engine.Common/Wrapper.hpp>

#include "Id.hpp"

namespace hg::editor::ui::rx {
	template <typename Type_>
	class Subject;

	template <typename Type_>
	struct Subscription {
		const Id id;
		ref<Subject<Type_>> subject;

		void unsubscribe() && {
			subject.unsubscribe(id);
		}
	};
}
