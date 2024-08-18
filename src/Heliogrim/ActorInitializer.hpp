#pragma once

#include <Engine.ACS/Registry.hpp>
#include <Engine.Common/Sal.hpp>
#include <Engine.Core/Session.hpp>
#include <Engine.Core/SessionState.hpp>

#include "Actor.hpp"
#include "ActorComponent.hpp"
#include "CachedActorPointer.hpp"

namespace hg {
	/**
	 * Forward Declaration
	 */
	class ActorComponent;
	class Session;

	class ActorInitializer {
	public:
		friend class ::hg::Session;

	public:
		using this_type = ActorInitializer;

	protected:
	public:
		ActorInitializer(cref<managed<void>> internal_) :
			_internal(internal_) {}

	public:
		~ActorInitializer() noexcept = default;

	private:
		/**
		 *
		 */
		managed<void> _internal;

	protected:
		// Warning: Temporary Solution
		[[nodiscard]] cref<engine::core::Session> getCoreSession() const noexcept {
			return *static_cast<const ptr<const engine::core::Session>>(_internal.get());
		}

	private:
	public:
		actor_guid _guid = invalid_actor_guid;

	public:
		template <std::derived_from<ActorComponent> Component>
		ptr<Component> createComponent(_Inout_ const ptr<Actor> actor_) const {

			/**/
			auto& registry { getCoreSession().getState().getRegistry() };

			auto* component = registry.acquireActorComponent<
				Component,
				CachedActorPointer,
				ptr<ActorComponent>
			>(actor_->guid(), { actor_->guid(), actor_ }, nullptr);
			assert(component != nullptr && "Failed to ensure successful created component.");

			/**
			 *
			 */
			assert(
				actor_->getRootComponent() == nullptr &&
				"Failed to ensure no existing root component at referenced actor."
			);
			actor_->addComponent(component);

			return component;
		}

		template <std::derived_from<ActorComponent> Component>
		ptr<Component> createSubComponent(_Inout_ const ptr<Actor> actor_, ptr<ActorComponent> parent_) const {

			/**/
			auto& registry { getCoreSession().getState().getRegistry() };

			auto* actor { actor_ ? actor_ : parent_->getOwner() };

			auto* component = registry.acquireActorComponent<
				Component,
				CachedActorPointer,
				ptr<ActorComponent>
			>(
				actor->guid(),
				CachedActorPointer { actor_->guid(), actor },
				ptr<ActorComponent> { parent_ }
			);
			assert(component != nullptr && "Failed to ensure successful created component.");

			/**
			 *
			 */
			// assert(actor->hasComponent(parent_), "Failed to ensure existing parent component at referenced actor.");
			actor_->addComponent(component);

			return component;
		}
	};
}
