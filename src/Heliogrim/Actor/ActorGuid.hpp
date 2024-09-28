#pragma once

#include <Engine.Common/Guid.hpp>
#include <Engine.Common/Wrapper.hpp>

namespace hg {
	struct ActorGuid : public Guid {
		using Guid::Guid;

		constexpr explicit ActorGuid(cref<Guid> guid_) noexcept :
			Guid(guid_) {}
	};
}
