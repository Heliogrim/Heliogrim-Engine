#pragma once

#include <Engine.Common/__macro.hpp>
#include <Engine.Common/Sal.hpp>

#include "Action.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Reflect;
#endif

namespace hg::editor {
	class macro_novtable SubAction :
		public InheritMeta<SubAction, Action> {
	protected:
		explicit SubAction(_In_ cref<Action> parentAction_);

	public:
		~SubAction() override = default;
	};
}
