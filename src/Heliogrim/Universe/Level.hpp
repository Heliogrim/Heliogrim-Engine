#pragma once

#include <Engine.Common/Optional.hpp>
#include <Engine.Common/Sal.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Managed/Rc.hpp>

#include "../Actor/Actor.hpp"
#include "../Async/Future.hpp"
#include "../Math/BuildIn.hpp"

namespace hg {
	class LevelAsset;
}

namespace hg::engine::core {
	class Level;
}

namespace hg {
	/**/

	struct LevelQueryOptions {};

	struct LevelLoadOptions {};

	struct LevelSaveOptions {};

	/**/

	class Level {
	public:
		using this_type = Level;

	public:
		Level() noexcept;

		explicit Level(mref<Arci<::hg::engine::core::Level>> internal_);

		Level(_In_ cref<Level> other_) noexcept;

		Level(_Inout_ mref<Level> other_) noexcept;

		~Level() noexcept;

	public:
		ref<Level> operator=(mref<Level>) noexcept;

		ref<Level> operator=(cref<Level>) = delete;

	private:
		Arci<::hg::engine::core::Level> _internal;

	public:
		[[nodiscard]] bool valid() const noexcept;

		[[nodiscard]] cref<decltype(_internal)> unwrap() const noexcept;

	public:
		/**
		 * Tries to add the given actor to this level
		 *
		 * @author Julius
		 * @date 01.12.2021
		 *
		 * @param actor_ The actor to add.
		 *
		 * @returns True if succeeded, otherwise false.
		 */
		bool addActor(_In_ ptr<Actor> actor_);

		/**
		 * Removes an actor from this level
		 *
		 * @author Julius
		 * @date 01.12.2021
		 *
		 * @param actor_ The actor to remove.
		 *
		 * @returns True if succeeded, otherwise false.
		 */
		bool removeActor(_In_ ptr<Actor> actor_);
	};

	/**
	 * Creates a new level with underlying object
	 *
	 * @author Julius
	 * @date 25.10.2021
	 *
	 * @returns A future, containing the newly created level if succeeded, otherwise nullptr.
	 */
	[[nodiscard, deprecated]] extern Future<ptr<Level>> CreateLevel() noexcept;

	/**/

	[[nodiscard]] extern Future<Level> CreateLevel() noexcept;

	[[nodiscard]] extern Future<Level> LoadLevel(
		_In_ cref<LevelAsset> levelAsset_,
		Opt<LevelLoadOptions> options_ = None
	) noexcept;

	extern Vector<Level> GetLevels(
		_In_ cref<Universe> universe_,
		Opt<LevelQueryOptions> options_ = None
	) noexcept;

	[[nodiscard]] extern Vector<Level> GetLevelsAt(
		_In_ cref<Universe> universe_,
		_In_ Vector3 location_,
		Opt<LevelQueryOptions> options_ = None
	) noexcept;

	[[nodiscard]] extern Future<bool> SaveLevel(
		_Inout_ ref<LevelAsset> levelAsset_,
		Opt<LevelSaveOptions> options_ = None
	) noexcept;

	[[nodiscard]] extern Future<bool> SaveLevel(
		_In_ cref<Level> level_,
		_Inout_ ref<LevelAsset> levelAsset_,
		Opt<LevelSaveOptions> options_ = None
	) noexcept;

	[[nodiscard]] extern Future<bool> Destroy(
		mref<Level> level_
	) noexcept;
}
