#pragma once

#include <Engine.ACS/Registry.hpp>
#include <Engine.Common/Sal.hpp>
#include <Engine.Core/Session.hpp>
#include <Engine.Core/SessionState.hpp>

#include "Actor/Actor.hpp"
#include "Component/CachedActorPointer.hpp"
#include "Component/HierarchyComponent.hpp"

namespace hg {
	/**
	 * Forward Declaration
	 */
	class HierarchyComponent;
	class Session;

	class ActorInitializer {
	public:
		friend class ::hg::Session;

	public:
		using this_type = ActorInitializer;

	protected:
	public:
		ActorInitializer(cref<SharedPtr<::hg::engine::core::Session>> internal_) :
			_internal(internal_) {}

	public:
		~ActorInitializer() noexcept = default;

	private:
		/**
		 *
		 */
		SharedPtr<::hg::engine::core::Session> _internal;

	protected:
		// Warning: Temporary Solution
		[[nodiscard]] cref<engine::core::Session> getCoreSession() const noexcept {
			return *static_cast<const ptr<const engine::core::Session>>(_internal.get());
		}

	private:
	public:
		actor_guid _guid = invalid_actor_guid;

	public:
		template <std::derived_from<HierarchyComponent> Component_>
		ptr<Component_> createComponent(_Inout_ const ptr<Actor> actor_) const {

			/**/
			auto& registry { getCoreSession().getState().getRegistry() };

			auto* component = registry.acquireActorComponent<
				Component_,
				CachedActorPointer,
				ptr<HierarchyComponent>
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

		template <std::derived_from<HierarchyComponent> Component_>
		ptr<Component_> createSubComponent(
			_Inout_ const ptr<Actor> actor_,
			_In_ ptr<HierarchyComponent> parent_
		) const {

			/**/
			auto& registry { getCoreSession().getState().getRegistry() };

			auto* actor { actor_ ? actor_ : parent_->getOwner() };

			auto* component = registry.acquireActorComponent<
				Component_,
				CachedActorPointer,
				ptr<HierarchyComponent>
			>(
				actor->guid(),
				CachedActorPointer { actor_->guid(), actor },
				ptr<HierarchyComponent> { parent_ }
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
