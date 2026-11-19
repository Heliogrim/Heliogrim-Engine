#pragma once

#include <Engine.Common/__macro.hpp>

#include "../UiService.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::reflow {
	class Window;
}

namespace hg::editor::ui {
	class ContextMenuProvider;
	class Menu;
}

namespace hg::editor::ui::service {
	class ContextService :
		public UiService {
	public:
		using this_type = ContextService;

	public:
		ContextService();

		~ContextService() override;

	private:
		Vector<SharedPtr<ContextMenuProvider>> _pushed;

	public:
		void push(mref<SharedPtr<ContextMenuProvider>> contextMenuProvider_);

		void pop(mref<SharedPtr<ContextMenuProvider>> contextMenuProvider_);

		[[nodiscard]] SharedPtr<Menu> buildContextMenu(ref<const engine::reflow::Window> window_) const;
	};
}
