#pragma once

#include "../Action.hpp"

namespace hg::editor {
	class ImportAction :
		public InheritMeta<ImportAction, Action> {
	public:
		using this_type = ImportAction;

	protected:
		constexpr ImportAction() noexcept = default;

	public:
		~ImportAction() noexcept override;
	};
}
