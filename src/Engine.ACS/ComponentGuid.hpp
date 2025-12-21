#pragma once

#include <Engine.Common/Guid.hpp>
#include <Engine.Common/Wrapper.hpp>

namespace hg {
	struct ComponentGuid : public Guid {
		using Guid::Guid;

		constexpr explicit ComponentGuid(ref<const Guid> guid_) noexcept :
			Guid(guid_) {}
	};
}
