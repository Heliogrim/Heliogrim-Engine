#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Math/Vector.hpp>
#include <Engine.Serialization/Access/Structure.hpp>

namespace hg::math {
	class Bounding;
}

namespace hg::engine::serialization::access {
	template <>
	void Structure<math::Bounding>::serialize(const math::Bounding* const self_, mref<RecordScopedSlot> slot_);

	template <>
	void Structure<math::fvec3>::serialize(const math::fvec3* const self_, mref<RecordScopedSlot> slot_);

	template <>
	void Structure<math::ivec3>::serialize(const math::ivec3* const self_, mref<RecordScopedSlot> slot_);

	template <>
	void Structure<math::uivec3>::serialize(const math::uivec3* const self_, mref<RecordScopedSlot> slot_);
}
