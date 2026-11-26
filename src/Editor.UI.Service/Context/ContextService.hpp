#pragma once

#include "../UiService.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::reflow {
	class Widget;
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

	public:
		/**
		 * Collects the `ContextMenuScope` providers from the target (itself included) upward, nearest first, until one consumes.
		 */
		[[nodiscard]] SharedPtr<Menu> buildContextMenu(
			ref<const engine::reflow::Window> window_,
			ref<engine::reflow::Widget> target_
		) const;
	};
}
