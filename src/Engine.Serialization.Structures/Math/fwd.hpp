#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
import Engine.Serialization;
#endif

namespace hg::engine::serialization::access {
	template <>
	void Structure<math::Bounding>::serialize(const math::Bounding& self_, mref<StructScopedSlot> record_);

	template <>
	void Structure<math::fvec3>::serialize(const math::vec3_t<float>& self_, mref<StructScopedSlot> record_);

	template <>
	void Structure<math::ivec3>::serialize(const math::vec3_t<int>& self_, mref<StructScopedSlot> record_);

	template <>
	void Structure<math::uivec3>::serialize(const math::vec3_t<unsigned>& self_, mref<StructScopedSlot> record_);
}
