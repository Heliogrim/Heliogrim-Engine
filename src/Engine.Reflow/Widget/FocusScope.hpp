#pragma once

#include "CompoundWidget.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::reflow {
	class EventRouter;

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

	public:
		[[nodiscard]] string getTag() const noexcept override;

		[[nodiscard]] FocusScopeKind kind() const noexcept;

		/**
		 * Last focused descendant, if still mounted below this scope.
		 */
		[[nodiscard]] sptr<Widget> remembered() const noexcept;

		void setContent(sptr<Widget> content_);
	};
}
