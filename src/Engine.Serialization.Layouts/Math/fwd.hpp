#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
import Engine.Serialization;
#endif

namespace hg::engine::serialization {
	template <>
	void DataLayout<math::fvec3>::describe();
}
