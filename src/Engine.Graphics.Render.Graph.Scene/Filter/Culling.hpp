#pragma once
#include <Engine.Common/__macro.hpp>

#include "../SceneWalkerFilter.hpp"

namespace hg::engine::render::graph {
	class macro_novtable CullingFilter :
		public InheritMeta<CullingFilter, SceneWalkerFilter> {};
}
