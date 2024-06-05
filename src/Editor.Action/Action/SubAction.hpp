#pragma once

#include <Engine.Common/Sal.hpp>
#include <Engine.Reflect/Inherit/InheritMeta.hpp>

#include "Action.hpp"

namespace hg::editor {
	class macro_novtable SubAction :
		public InheritMeta<SubAction, Action> {
	protected:
		explicit SubAction(_In_ cref<Action> parentAction_);

	public:
		~SubAction() override = default;
	};
}
