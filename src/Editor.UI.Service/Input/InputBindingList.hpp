#pragma once

#include <Engine.Common/Optional.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/Managed/Rc.hpp>

#include "InputBinding.hpp"

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
