#pragma once

#include <span>
#include <Engine.ACS/Handle/Handle.hpp>
#include <Engine.Common/Optional.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/DenseSet.hpp>
#include <Engine.Common/Managed/AtomicRefCountedIntrusive.hpp>
#include <Engine.Common/Managed/Rc.hpp>
#include <Engine.Common/Math/Bounding.hpp>
#include <Engine.Common/Memory/Pointer.hpp>
#include <Engine.Serialization/Access/__fwd.hpp>
#include <Engine.Serialization/Layout/DataLayout.hpp>
#include <Heliogrim/Actor/Actor.hpp>

#include "LevelGuid.hpp"

namespace hg {
	struct AssetGuid;
}

namespace hg::engine::assets {
	class LevelAsset;
}

namespace hg::engine::core {
	class Level :
		public ArcFromThis<Level> {
	public:
		template <typename>
		friend class ::hg::engine::serialization::DataLayout;

		ACCESS_LAYOUT
		ACCESS_STRUCTURE

	public:
		using this_type = Level;

	public:
		Level(mref<LevelGuid> guid_) noexcept;

		Level(ref<const assets::LevelAsset> asset_) noexcept;

		~Level();

	private:
		LevelGuid _guid;

		math::Bounding _bounding;
		Opt<ref<engine::core::Universe>> _universe;

	public:
		[[nodiscard]] bool isRootLike() const noexcept;

		[[nodiscard]] ref<const LevelGuid> guid() const noexcept;

		[[nodiscard]] ref<const AssetGuid> assetGuid() const noexcept;

		[[nodiscard]] cref<math::Bounding> getBounding() const noexcept;

		void setBounding(cref<math::Bounding> bounding_) noexcept;

		[[nodiscard]] Opt<ref<engine::core::Universe>> getUniverse() const noexcept;

		void setOwnerUniverse(mref<Opt<ref<engine::core::Universe>>> owner_);

	private:
		// Space of this level's cluster regions and handles; acquired with the first archetype actor.
		acs::SpaceId _space = acs::detachedSpace;

		DenseSet<owner_ptr<Actor>> _actors;

	public:
		/**
		 * @returns the space of this level, acquired on first use
		 */
		[[nodiscard]] acs::SpaceId space();

		/**
		 * Mounts an actor; an archetype actor from another space (e.g. detached) is relocated into this level's region.
		 *
		 * @returns the actor's address after mounting
		 */
		ptr<Actor> addActor(_In_ mref<VolatileActor<>> actor_);

		[[nodiscard]] std::span<const owner_ptr<Actor>> getActors() const noexcept;

		/**
		 * Unmounts an actor; an archetype actor is relocated to the detached space, so it outlives this level.
		 */
		[[nodiscard]] Opt<VolatileActor<>> removeActor(nmpt<Actor> actor_);
	};

	/**/

	[[nodiscard]] extern Arci<Level> make_root_like_level();
}
