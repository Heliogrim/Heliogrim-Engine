#pragma once

#include <Engine.Common/__macro.hpp>

#include "InputBinding.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::editor::ui::service {
	class InputBindingList {
	public:
		using this_type = InputBindingList;

	private:
		Vector<InputBinding> _bindings;

	public:
		bool addBinding(mref<InputBinding> binding_);

	public:
		[[nodiscard]] Opt<Arci<Action>> invoke(ref<const InlineAutoArray<u32, 4>> keys_) const;
	};
}
