#pragma once

#include <Engine.Common/Math/Vector.hpp>
#include <Engine.Serialization/Layout/DataLayout.hpp>

namespace hg::engine::serialization {
	template <>
	void DataLayout<math::fvec3>::describe();
}
