#pragma once
#include "../SceneWalkerFilter.hpp"

#include <Engine.Common/__macro.hpp>

namespace hg::engine::render::graph {
	class macro_novtable CullingFilter :
		public InheritMeta<CullingFilter, SceneWalkerFilter> {};
}
