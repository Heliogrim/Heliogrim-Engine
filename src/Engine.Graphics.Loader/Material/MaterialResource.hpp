#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Graphics.Material;
import Engine.Resource;
#endif

namespace hg::engine::graphics {
	typedef resource::Resource<material::Material> MaterialResource;
}