#pragma once

#include <Engine.Assets/AssetGuid.hpp>
#include <Engine.Common/Sal.hpp>
#include <Engine.Common/Wrapper.hpp>

#include "../Actor/Actor.hpp"
#include "../Async/Future.hpp"

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
		_Success_(return == true) bool addLevel(_In_ cref<Level> level_);

		_Success_(return == true) bool removeLevel(_In_ cref<Level> level_);

	public:
		/**
		 * Tries to add the given actor to this universe
		 *
		 * @author Julius
		 * @date 01.12.2021
		 *
		 * @param actor_ The actor to add.
		 *
		 * @returns A reference to the added actor (managed).
		 */
		ptr<Actor> addActor(_Inout_ mref<VolatileActor<>> actor_);

		template <class ActorType_>
		ptr<ActorType_> addActor(_Inout_ mref<VolatileActor<ActorType_>> actor_) {
			return static_cast<ptr<ActorType_>>(addActor(static_cast<mref<VolatileActor<Actor>>>(std::move(actor_))));
		}

		/**
		 * Removes an actor from this universe
		 *
		 * @author Julius
		 * @date 01.12.2021
		 *
		 * @param actor_ A reference of the actor to remove.
		 *
		 * @returns A volatile actor handle of the removed actor.
		 */
		VolatileActor<> removeActor(_In_ ptr<Actor> actor_);

		/**
		 * Drops an actor instance from this level
		 *
		 * @param actor_ The actor instance to drop.
		 */
		void dropActor(_Inout_ mref<ptr<Actor>> actor_);
	};

	/**/

	[[nodiscard]] extern Future<Universe> CreateUniverse() noexcept;

	[[nodiscard]] extern Universe GetUniverse(
		_In_ cref<Session> session_
	) noexcept;

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
	[[nodiscard]] extern Future<Universe> GetUniverse(_In_ cref<asset_guid> guid_) noexcept;

	extern Future<bool> Destroy(
		mref<Universe> universe_
	);

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
	extern void SetUniverse(_Inout_ ref<Session> session_, _In_ cref<Universe> universe_);

	/* Warning: Experimental Feature */
	/**
	 * Transition from the session's current universe to another one
	 *
	 * @author Julius
	 * @date 10.12.2022
	 *
	 * @see ::hg::SetUniverse(...)
	 */
	extern void TransitionToUniverse(_Inout_ ref<Session> session_, _In_ cref<Universe> universe_);
}
