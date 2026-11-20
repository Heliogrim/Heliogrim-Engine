#pragma once

#include <Engine.Accel.Lang/Intermediate.hpp>

#include "ProfileDefinition.hpp"
#include "ProfileUsagePattern.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::accel {
	class EffectProfile {
	public:
		string _name;

	public:
		Vector<ProfileDefinition> _definitions;
		Vector<lang::Intermediate> _modules;
		/* toggle values the profile fixes for its ir stages, sorted by toggle name */
		lang::ir::ToggleSet _toggles;

	private:
	public:
		ProfileUsagePattern _usagePattern;

	public:
		[[nodiscard]] cref<ProfileUsagePattern> getUsagePattern() const noexcept;
	};
}
