#pragma once

#include <Engine.Common/Wrapper.hpp>

namespace hg::engine::storage {
	template <typename ManagedType_>
	struct Io {
		using this_type = Io<ManagedType_>;

		[[nodiscard]] ptr<ManagedType_> operator->();
	};
}
