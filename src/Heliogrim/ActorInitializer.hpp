#pragma once

#include <memory>
#include <Engine.ACS/Registry.hpp>
#include <Engine.Asserts/Asserts.hpp>
#include <Engine.Common/Move.hpp>
#include <Engine.Common/Optional.hpp>
#include <Engine.Common/Sal.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>

#include "Component/CachedActorPointer.hpp"
#include "Component/HierarchyComponent.hpp"

namespace hg {
	/**
	 * Forward Declaration
	 */
	class Actor;
	class HierarchyComponent;
	class Session;

	template <typename Type_, typename... Args_>
	concept IsConstructibleComponent = CompleteType<Type_> &&
		std::derived_from<Type_, HierarchyComponent> &&
		std::is_constructible_v<Type_, mref<ComponentGuid>, mref<CachedActorPointer>, mref<ptr<HierarchyComponent>>, Args_&&...>;

	class ActorInitializer {
	public:
		friend class ::hg::Session;

	public:
		using this_type = ActorInitializer;

	protected:
	public:
		explicit ActorInitializer(ref<::hg::engine::acs::Registry> internal_) :
			_internal(std::addressof(internal_)) {}

	public:
		~ActorInitializer() noexcept = default;

	private:
		/**
		 *
		 */
		nmpt<::hg::engine::acs::Registry> _internal;

	private:
	public:
		ActorGuid _guid = invalid_actor_guid;

	public:
		template <
			IsConstructibleComponent Component_,
			std::derived_from<Actor> Actor_,
			typename AddComponentOptions_ = Actor_::AddComponentOptions>
		ptr<Component_> createComponent(_Inout_ ref<Actor_> actor_, _In_ mref<AddComponentOptions_> options_, _In_ auto&&... args_) const
			requires std::is_constructible_v<
				Component_, mref<ComponentGuid>, mref<CachedActorPointer>, mref<ptr<HierarchyComponent>>, decltype(args_)&&...
			> {

			auto guid = generate_component_guid();
			auto component = _internal->acquireActorComponent<Component_, CachedActorPointer, ptr<HierarchyComponent>>(
				::hg::move(guid),
				{ actor_.guid(), std::addressof(actor_) },
				options_.parent.has_value() ? std::addressof(options_.parent.value()) : nullptr,
				std::forward<decltype(args_)>(args_)...
			);
			::hg::assertrt(
				/* Check: Failed to ensure successful created component. */
				component != nullptr
			);

			/**/

			auto* const result = component.get();
			if (options_.parent == None) {
				::hg::assertrt(
					/* Check: Failed to ensure no existing root component at referenced actor. */
					not actor_.getRootComponent().has_value()
				);
				actor_.addComponent(::hg::move(component), options_);

			} else {
				::hg::assertd(
					/* Check: Failed to ensure existing parent component at referenced actor. */
					actor_.getComponents().contains(std::addressof(options_.parent.value()))
				);
				actor_.addComponent(::hg::move(component), options_);
			}

			return result;
		}
	};
}