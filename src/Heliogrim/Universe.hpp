#pragma once

#include <Engine.Assets/AssetGuid.hpp>
#include <Engine.Common/Sal.hpp>
#include <Engine.Common/Wrapper.hpp>

#include "Actor.hpp"
#include "Future.hpp"

namespace hg {
	class Level;
}

namespace hg::engine::core {
	class Universe;
}

namespace hg {
	class Universe {
	public:
		using this_type = Universe;

	public:
		/**
		 * Default constructor
		 *
		 * @author Julius
		 * @date 24.10.2021
		 */
		Universe();

		explicit Universe(mref<SharedPtr<::hg::engine::core::Universe>> internal_);

		Universe(cref<this_type> other_) = default;

		Universe(mref<this_type> other_) noexcept = default;

		/**
		 * Destructor
		 *
		 * @author Julius
		 * @date 24.10.2021
		 */
		~Universe() noexcept = default;

	public:
		/**
		 * Copy Assignment operator
		 *
		 * @author Julius
		 * @date 24.10.2021
		 *
		 * @returns A shallow copy of this.
		 */
		ref<Universe> operator=(cref<Universe> other_) = default;

		/**
		 * Move Assignment operator
		 *
		 * @author Julius
		 * @date 24.10.2021
		 *
		 * @returns A shallow copy of this.
		 */
		ref<Universe> operator=(mref<Universe> other_) noexcept = default;

	public:
		[[nodiscard]] bool valid() const noexcept;

	private:
		/**
		 * Internal
		 */
		SharedPtr<::hg::engine::core::Universe> _internal;

	public:
		/**
		 * Gets the internal state
		 *
		 * @author Julius
		 * @date 04.12.2022
		 *
		 * @returns A reference to the internal managed universe representation
		 */
		[[nodiscard]] cref<decltype(_internal)> unwrap() const noexcept;

	public:
		/**
		 * Tries to add the given level to this universe
		 *
		 * @author Julius
		 * @date 24.10.2021
		 *
		 * @param  level_ The level.
		 *
		 * @returns True if it succeeds, false if it fails.
		 */
		_Success_(return == true) bool addLevel(_In_ const ptr<Level> level_);

		/**
		 * Removes the level from this universe
		 *
		 * @author Julius
		 * @date 24.10.2021
		 *
		 * @param  level_ The level.
		 *
		 * @returns True if it succeeds, false if it fails.
		 */
		_Success_(return == true) bool removeLevel(_In_ const ptr<Level> level_);

	public:
		/**
		 * Tries to add the given actor to this universe
		 *
		 * @author Julius
		 * @date 01.12.2021
		 *
		 * @param actor_ The actor to add.
		 *
		 * @returns True if succeeded, otherwise false.
		 */
		bool addActor(const ptr<Actor> actor_);

		/**
		 * Removes an actor from this universe
		 *
		 * @author Julius
		 * @date 01.12.2021
		 *
		 * @param actor_ The actor to remove.
		 *
		 * @returns True if succeeded, otherwise false.
		 */
		bool removeActor(const ptr<Actor> actor_);
	};

	/**
	 * Creates a new universe instance
	 *
	 * @author Julius
	 * @date 25.11.2021
	 *
	 * @returns A future, containing the newly created universe
	 */
	[[nodiscard]] extern Future<Universe> CreateUniverse() noexcept;

	/**
	 * Query the session's current universe
	 *
	 * @author Julius
	 * @date 09.12.2022
	 *
	 * @param session_ The session where to lookup the current universe.
	 *
	 * @returns A representation for the currently internal stored universe.
	 */
	[[nodiscard]] extern Universe GetUniverse(cref<Session> session_) noexcept;

	/**
	 * Query the current primary core session's universe representation
	 *
	 * @details A convenient shortcut for obtaining the current primary
	 *  core session's universe instance.
	 *
	 * @author Julius
	 * @date 25.11.2021
	 *
	 * @returns A representation for the current internal universe.
	 */
	[[nodiscard]] extern Universe GetUniverse() noexcept;

	/**
	 * Gets a universe instance by it's guid
	 *
	 * @author Julius
	 * @date 25.11.2021
	 *
	 * @param guid_ The guid of the universe to lookup.
	 *
	 * @returns A future, containing a universe instance
	 */
	[[nodiscard]] extern Future<Universe> GetUniverse(cref<asset_guid> guid_) noexcept;

	/**
	 * Destroys the given universe and it's implicit resources
	 *
	 * @author Julius
	 * @date 25.11.2021
	 *
	 * @param universe_ The universe instance to destroy.
	 *
	 * @returns A future, representing whether the universe was successfully destroyed.
	 */
	extern Future<bool> Destroy(mref<Universe> universe_);

	/* Warning: Experimental Feature */
	/**
	 * Set's the universe of a core session
	 *
	 * @note The universe *must* have been created with a engine wide propagation,
	 *  otherwise this would result in ub.
	 *
	 * @warning This method is currently unsafe. The implicit internal changes
	 *  aspecially at the session's universe-context may result in memory corruption or
	 *  modify-on-read behaviour. Further this change is required to propagate through
	 *  whole engine and modules, therefore providing an endless amount of conflict points.
	 *
	 * @note We might want this to return a future result, due to the most likely asynchronous fix
	 *  to the described problems. This might result in a full schedule tick to complete the
	 *  required changes.
	 *
	 * @author Julius
	 * @date 10.12.2022
	 *
	 * @param session_ The session where to set the current universe
	 * @param universe_ The universe to use next.
	 */
	extern void SetUniverse(cref<Session> session_, cref<Universe> universe_);

	/* Warning: Experimental Feature */
	/**
	 * Transition from the session's current universe to another one
	 *
	 * @author Julius
	 * @date 10.12.2022
	 *
	 * @see ::hg::SetUniverse(...)
	 */
	extern void TransitionToUniverse(cref<Session> session_, cref<Universe> universe_);
}
