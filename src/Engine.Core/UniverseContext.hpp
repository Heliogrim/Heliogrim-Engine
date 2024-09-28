#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>
#include <Engine.Common/Memory/SharedPointer.hpp>

#include "UniverseType.hpp"

namespace hg::engine::core {
	class UniverseContext {
	public:
		using this_type = UniverseContext;

	public:
		explicit UniverseContext(ref<class Session> owner_);

		UniverseContext(cref<this_type>) = delete;

		UniverseContext(mref<this_type>) noexcept = delete;

		~UniverseContext();

	private:
		nmpt<class Session> _owner;

		// TODO: Check whether we want a atomic_shared_pointer right here, cause we might modify it concurrently
		SharedPtr<class Universe> _universe;
		UniverseType _universeType;

	public:
		[[nodiscard]] ref<class Session> getOwner() const noexcept;

		[[nodiscard]] cref<SharedPtr<Universe>> getCurrentUniverse() const noexcept;

		void setCurrentUniverse(mref<SharedPtr<Universe>> nextUniverse_);

		[[nodiscard]] UniverseType getUniverseType() const noexcept;
	};
}
