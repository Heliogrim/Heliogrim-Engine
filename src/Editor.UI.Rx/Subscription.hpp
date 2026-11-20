#pragma once

#include "Id.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

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
