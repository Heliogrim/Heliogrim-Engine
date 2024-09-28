#include "fwd.hpp"

using namespace hg;

namespace hg::engine::serialization {
	template <>
	void DataLayout<math::fvec3>::describe() {

		using namespace ::hg::engine::serialization::layout;

		defineValue<LayoutDefineValueType::eFloat>(offsetof(math::fvec3, x));
		defineValue<LayoutDefineValueType::eFloat>(offsetof(math::fvec3, y));
		defineValue<LayoutDefineValueType::eFloat>(offsetof(math::fvec3, z));
	}
}
