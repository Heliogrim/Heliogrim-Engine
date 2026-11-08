#pragma once

#include <type_traits>
#include <Engine.Common/Types.hpp>
#include <Engine.Common/Variant.hpp>

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
