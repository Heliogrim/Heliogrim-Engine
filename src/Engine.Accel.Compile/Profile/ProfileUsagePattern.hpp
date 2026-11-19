#pragma once
#include <Engine.Accel.Lang/Rep/Symbol.hpp>
#include <Engine.Common/__macro.hpp>
#include <Engine.Graphics.RenderGraph/Relation/Description.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::accel {
	/**
	 * Used for contextual resolved resources (i.e. camera data)
	 */
	struct ContextUsagePattern {
		/**/
		Vector<std::pair<smr<const render::graph::Description>, accel::lang::SymbolId>> pairs;
		// i.e. pair< SceneViewDescription <-> SymbolId >
	};

	struct DynamicUsagePattern {
		/**/
		Vector<std::pair<smr<const render::graph::Description>, accel::lang::SymbolId>> pairs;
		// i.e. pair< MeshDescription <-> SymbolId >
	};

	struct StaticUsagePattern {
		// Note: Currently unsupported
	};

	/**/
	struct ProfileUsagePattern {
		ContextUsagePattern ctx;
		DynamicUsagePattern dyn;
		StaticUsagePattern st;
	};
}
