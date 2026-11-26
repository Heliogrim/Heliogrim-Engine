#pragma once

#include "CompoundWidget.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::reflow {
	class EventRouter;
	class FocusScope;

	/**
	 * Nearest focus scope above a widget, provided by every `FocusScope` to its subtree.
	 */
	struct FocusScopeContext {
		constexpr static bool isStatic = true;
		nmpt<FocusScope> scope;
	};

	enum class FocusScopeKind : u8 {
		eWindow,
		ePanel,
		ePopup,
		eDialog
	};

	/**
	 * Single child boundary for tab order and click focus fallback, transparent to layout.
	 */
	class FocusScope final :
		public CompoundWidget {
	private:
		friend class EventRouter;

	public:
		using this_type = FocusScope;

	public:
		explicit FocusScope(const FocusScopeKind kind_);

		~FocusScope() override;

	private:
		FocusScopeKind _kind;
		wptr<Widget> _remembered;
		FocusScopeContext _context;

	public:
		[[nodiscard]] string getTag() const noexcept override;

		[[nodiscard]] FocusScopeKind kind() const noexcept;

		/**
		 * Last focused descendant, if still mounted below this scope.
		 */
		[[nodiscard]] sptr<Widget> remembered() const noexcept;

		void setContent(const sptr<Widget> content_);

		[[nodiscard]] ptr<void> providedContext(const type_id contextType_) noexcept override;
	};
}
