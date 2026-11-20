#pragma once

#include <Engine.Serialization/Layout/DataLayout.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::serialization {
	template <>
	void DataLayout<math::fvec3>::describe();
}
