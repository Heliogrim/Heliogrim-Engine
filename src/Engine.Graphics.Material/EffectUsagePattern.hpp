#pragma once
#include <Engine.Graphics.Render.Predefined/Buffer/ViewBuffer.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Accel.Lang;
import Engine.Common;
import Engine.Graphics.RenderGraph;
import Engine.Graphics.Types;
#endif

namespace hg::engine::graphics::material {
	/**
	 * Used for contextual resolved resources (i.e. camera data)
	 */
	struct ContextUsagePattern {
		/**/
		Vector<std::pair<smr<const render::graph::Description>, accel::lang::SymbolId>> pairs;
		// i.e. pair< SceneViewDescription <-> SymbolId >
	};

	/**
	 * Used for dynamic material bindings or dynamic substituted resources
	 */
	struct DynamicUsagePattern {
		/**/
	};

	/**
	 * Used for static material bindings to target effects
	 */
	struct StaticUsagePattern {
		Vector<std::pair<ParameterIdentifier, accel::lang::SymbolId>> pairs;
	};

	/**/
	struct EffectUsagePattern {
		ContextUsagePattern ctx;
		DynamicUsagePattern dyn;
		StaticUsagePattern st;
	};
}
