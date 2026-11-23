#pragma once

#include <Engine.Common/Sal.hpp>
#include <Engine.Serialization/Access/__fwd.hpp>

#include "LevelGuid.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Diagnostics;
import Engine.ACS;
import Engine.Common;
import Engine.Serialization;
#endif

namespace hg::engine::resource {
	class Archive;
}

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

		bool _unknownData = false;

	public:
		[[nodiscard]] bool isRootLike() const noexcept;

		[[nodiscard]] ref<const LevelGuid> guid() const noexcept;

		[[nodiscard]] ref<const AssetGuid> assetGuid() const noexcept;

		[[nodiscard]] cref<math::Bounding> getBounding() const noexcept;

		void setBounding(cref<math::Bounding> bounding_) noexcept;

		[[nodiscard]] Opt<ref<engine::core::Universe>> getUniverse() const noexcept;

		void setOwnerUniverse(mref<Opt<ref<engine::core::Universe>>> owner_);

		/* Set when a load met data this build does not understand (e.g. a skipped actor); such a level must not be saved back. */
		[[nodiscard]] bool carriesUnknownData() const noexcept;

		void markUnknownData() noexcept;

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

	/**
	 * Save boundary check: refuses (`save-refused`, one note per marked object) when the level or any of its actors carries unknown data.
	 */
	[[nodiscard]] extern Result<void, diag::Diagnostics> checkSaveAllowed(ref<const Level> level_);

	/**
	 * Engine load boundary: hydrates a level record root all-or-nothing into `into_` (a fresh root-like level when null); errors go to the sink and are returned.
	 */
	[[nodiscard]] extern Result<Arci<Level>, engine::diag::Diagnostics> loadLevel(
		_Inout_ ref<engine::resource::Archive> archive_,
		_In_ const StringView url_,
		_Inout_ ref<engine::diag::DiagnosticSink> sink_,
		_In_ mref<Arci<Level>> into_ = nullptr
	);
}
