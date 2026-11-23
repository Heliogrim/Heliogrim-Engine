#pragma once

#include <Engine.Graphics.Material/Material.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Resource;
#endif

namespace hg::engine::graphics {
	typedef resource::Resource<material::Material> MaterialResource;
}