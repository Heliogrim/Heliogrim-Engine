#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg {
	struct Broken {
		enum class Reason : u8 {
			eDropped,
			eRejected
		} reason;
	};

	template <typename E_>
	using Fail = std::conditional_t<std::is_same_v<E_, Broken>, Broken, Variant<E_, Broken>>;
}
