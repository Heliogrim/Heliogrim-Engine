#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>

#include "WorldType.hpp"

namespace hg::engine::core {
	class WorldContext {
	public:
		using this_type = WorldContext;

	public:
		explicit WorldContext(nmpt<class Session> owner_);

		WorldContext(cref<this_type>) = delete;

		WorldContext(mref<this_type>) noexcept = delete;

		~WorldContext();

	private:
		nmpt<class Session> _owner;

		// TODO: Check whether we want a atomic_shared_pointer right here, cause we might modify it concurrently
		sptr<class World> _world;
		WorldType _worldType;

	public:
		[[nodiscard]] nmpt<class Session> getOwner() const noexcept;

		[[nodiscard]] cref<sptr<World>> getCurrentWorld() const noexcept;

		void setCurrentWorld(cref<sptr<World>> nextWorld_);

		[[nodiscard]] WorldType getWorldType() const noexcept;
	};
}
