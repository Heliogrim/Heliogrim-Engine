#pragma once

#include "ThemeContext.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::reflow::theming {
	class ThemeProvisioner {
	public:
		using this_type = ThemeProvisioner;

	protected:
		ThemeProvisioner() = default;

	public:
		virtual ~ThemeProvisioner() = default;

	protected:
		ThemeContext _localContext;

		[[nodiscard]] ref<ThemeContext> getLocalContext();

		void setLocalContextParent(Opt<ref<const ThemeContext>> parent_);

	public:
		/**
		 * @throws If no valid context could be found (e.g. due to missing hierarchy mounting or timing problems)
		 * @returns a valid const reference to the effective theme context.
		 */
		[[nodiscard]] ref<const ThemeContext> getContext() const;

	public:
		/**
		 * @details Receiving an empty optional value results from incorrect state, and/or unmounted or timing related problems.
		 * @returns a const reference to the parent theme provisioner if one exists, otherwise an empty state.
		 */
		[[nodiscard]] virtual Opt<ref<const ThemeProvisioner>> findParentProvisioner() const noexcept = 0;
	};
}
