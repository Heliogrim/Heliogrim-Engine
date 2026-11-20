#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::serialization::access {
	template <class Type_, class Scheme_>
	class Structure;
}

namespace hg::engine::serialization {
	class RecordScopedSlot;
}

namespace hg::engine::serialization {
	/**/

	template <CompleteType Type_>
	struct Scheme {};

	/**/

	template <template <typename> typename SchemeType_, typename TargetType_>
	concept IsScheme = std::derived_from<SchemeType_<TargetType_>, Scheme<TargetType_>>;
}
