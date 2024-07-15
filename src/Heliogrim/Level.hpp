#pragma once

#include <Engine.Common/Sal.hpp>

#include "Actor.hpp"
#include "Inbuilt.hpp"

namespace hg {
	class Level :
		public InheritBase<Level> {
	public:
		using this_type = Level;
		using underlying_type = InheritBase<Level>;

	public:
		/**
		 * Default constructor
		 *
		 * @author Julius
		 * @date 24.10.2021
		 */
		Level() noexcept;

		/**
		 * Copy Constructor
		 *
		 * @author Julius
		 * @date 24.10.2021
		 *
		 * @param  other_ The other.
		 */
		Level(_In_ cref<Level> other_) noexcept;

		/**
		 * Move Constructor
		 *
		 * @author Julius
		 * @date 24.10.2021
		 *
		 * @param  other_ The other.
		 */
		Level(_Inout_ mref<Level> other_) noexcept;

		/**
		 * Destructor
		 *
		 * @author Julius
		 * @date 24.10.2021
		 */
		~Level() noexcept override;

	public:
		/**
		 * Copy Assignment
		 *
		 * @author Julius
		 * @date 24.10.2021
		 *
		 * @returns A shallow copy of this.
		 */
		ref<Level> operator=(cref<Level>) = delete;

		/**
		 * Move Assignment
		 *
		 * @author Julius
		 * @date 24.10.2021
		 *
		 * @returns A shallow copy of this.
		 */
		ref<Level> operator=(mref<Level>) noexcept = delete;

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
		bool addActor(const ptr<Actor> actor_);

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
		bool removeActor(const ptr<Actor> actor_);
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

	/**
	 * Gets a collection of the current active Levels
	 *
	 * @author Julius
	 * @date 25.11.2021
	 *
	 * @returns A collection containing the pointers to and amount of active levels.
	 */
	[[nodiscard, deprecated]] extern Vector<ptr<Level>> GetLevels() noexcept;

	/**
	 * Gets the primary active level at the given location
	 *
	 * @author Julius
	 * @date 25.11.2021
	 *
	 * @param location_ The position where to look for an active level.
	 *
	 * @returns The pointer to the active level instance, otherwise nullptr
	 */
	[[nodiscard, deprecated]] extern ptr<Level> GetLevelAt(Vector3 location_) noexcept;

	/**
	 * Destroy the given level and it's implicit resources
	 *
	 * @author Julius
	 * @date 25.10.2021
	 *
	 * @param  level_ The Level instance to destroy.
	 *
	 * @returns A future, representing whether the entity was successfully destroyed.
	 */
	[[nodiscard, deprecated]] extern Future<bool> Destroy(mref<ptr<Level>> level_) noexcept;
}
