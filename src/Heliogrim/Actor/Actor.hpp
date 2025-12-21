#pragma once

#include <Engine.ACS/ActorModule.hpp>
#include <Engine.ACS/Traits.hpp>
#include <Engine.Common/Optional.hpp>
#include <Engine.Common/Sal.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/CompactArray.hpp>
#include <Engine.Common/Collection/Set.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.Reflect/IsType.hpp>
#include <Engine.Reflect/Reflect.hpp>
#include <Engine.Reflect/Inherit/InheritBase.hpp>
#include <Engine.Serialization/Access/__fwd.hpp>

#include "../ActorInitializer.hpp"
#include "../Async/Future.hpp"
#include "../Async/Traits.hpp"

namespace hg {
	/**
	 * Forward Declaration
	 */
	struct CachedActorPointer;
	class HierarchyComponent;
	struct SerializedActor;

	class IComponentRegisterContext;
	class Session;
	class Universe;

	using ActorClass = ::hg::MetaClass;

	/**
	 * The base class of every actor
	 *
	 * @author Julius
	 * @date 25.11.2021
	 */
	class Actor :
		public InheritBase<Actor> {
	public:
		ACCESS_LAYOUT
		ACCESS_STRUCTURE

	public:
		using this_type = Actor;

		inline constexpr static type_id typeId { "Actor"_typeId };

	public:
		/**
		 * Constructor
		 *
		 * @author Julius
		 * @date 25.11.2021
		 */
		Actor(cref<ActorInitializer> initializer_) noexcept;

		Actor (cref<this_type>) = delete;

		Actor(mref<this_type>) noexcept = delete;

		/**
		 * Destructor
		 *
		 * @author Julius
		 * @date 25.11.2021
		 */
		~Actor() noexcept override;

	public:
		/**
		 * Move Assignment
		 *
		 * @author Julius
		 * @date 25.11.2021
		 */
		ref<Actor> operator=(mref<Actor>) noexcept = delete;

		/**
		 * Copy Assignment
		 *
		 * @author Julius
		 * @date 25.11.2021
		 */
		ref<Actor> operator=(cref<Actor>) = delete;

	private:
		ActorGuid _guid;

	public:
		/**
		 * Gets the unique identifier of this actor object
		 *
		 * @author Julius
		 * @date 25.11.2021
		 *
		 * @returns The actor_guid of this actor object.
		 */
		[[nodiscard]] ActorGuid guid() const noexcept;

		void unsafe_set_guid(cref<ActorGuid> guid_);

	protected:
		Opt<ref<engine::core::Universe>> _universe;
		Opt<ref<HierarchyComponent>> _rootComponent;

	public:
		[[nodiscard]] Opt<ref<HierarchyComponent>> getRootComponent() const noexcept;

	private:
		CompactSet<ptr<HierarchyComponent>> _components;

	private:
		template <typename Fnc_>
		void eachComponent(Fnc_&& fnc_) const {
			/**
			 *
			 */
			for (auto& entry : _components) {
				fnc_(entry);
			}
		}

		void scheduleComponentRemoval(mref<ptr<HierarchyComponent>> component_);

		void scheduleComponentRelease(mref<ptr<LogicComponent>> component_) const;

	public:
		struct AddComponentOptions {
			bool autoRegister : 1 = true;
			Opt<ref<HierarchyComponent>> parent;
		};

		template <IsConstructibleComponent Component_, typename Arg0_, typename... Args_>
			requires IsConstructibleComponent < Component_

		,
		Arg0_&&
		,
		Args_&&
		...
		>
		Opt<ref<Component_>> addComponent(_In_ mref<AddComponentOptions> options_, Arg0_&& arg0_, Args_&&... args_) {

			// TODO: Rework
			auto initializer = ActorInitializer { *engine::Engine::getEngine()->getActors()->getRegistry() };
			return Some<ref<Component_>>(
				*initializer.createComponent<Component_>(
					*this,
					::hg::move(options_),
					std::forward<Arg0_>(arg0_),
					std::forward<Args_>(args_)...
				)
			);
		}

		template <IsConstructibleComponent Component_>
		Opt<ref<Component_>> addComponent(_In_ mref<AddComponentOptions> options_) {

			// TODO: Rework
			auto initializer = ActorInitializer { *engine::Engine::getEngine()->getActors()->getRegistry() };
			return Some<ref<Component_>>(*initializer.createComponent < Component_ > (*this, ::hg::move(options_)));
		}

		template <IsConstructibleComponent Component_, typename Arg0_, typename... Args_>
			requires (not
		std::is_same_v<::hg::meta::peeled_t<Arg0_>, AddComponentOptions>
		)
		&&
		IsConstructibleComponent
		<
		Component_
		,
		Arg0_&&
		,
		Args_&&
		...
		>
		Opt<ref<Component_>> addComponent(Arg0_&& arg0_, Args_&&... args_) {
			if (_rootComponent.is_null()) {
				return addComponent<Component_, Arg0_, Args_...>(
					{ .parent = None },
					::hg::forward<Arg0_>(arg0_),
					hg::forward<Args_>(args_)...
				);
			}
			return addComponent<Component_, Arg0_, Args_...>(
				{ .parent = _rootComponent },
				::hg::forward<Arg0_>(arg0_),
				hg::forward<Args_>(args_)...
			);
		}

		template <IsConstructibleComponent Component_>
		Opt<ref<Component_>> addComponent() {
			if (_rootComponent.is_null()) {
				return addComponent<Component_>({ .parent = None });
			}
			return addComponent<Component_>({ .parent = _rootComponent });
		}

		void addComponent(_In_ mref<VolatileComponent<HierarchyComponent>> component_, _In_ Opt<ref<const AddComponentOptions>> options_);

		[[nodiscard]] cref<CompactSet<ptr<HierarchyComponent>>> getComponents() const noexcept;

		template <typename Selector_>
		[[nodiscard]] CompactArray<ptr<HierarchyComponent>> selectComponents(Selector_&& selector_) const {
			CompactArray<ptr<HierarchyComponent>> result {};
			eachComponent(
				[&](const ptr<HierarchyComponent> component_) {
					if (selector_(component_)) {
						result.emplace_back(component_);
					}
				}
			);

			return result;
		}

		template <IsConstructibleComponent Component_>
		[[nodiscard]] AutoArray<VolatileComponent<HierarchyComponent>> removeComponents(ref<Component_> component_) {

			auto result = AutoArray<VolatileComponent<HierarchyComponent>> {
				VolatileComponent < Component_ > { std::addressof(component_) }
			};

			for (auto parentIndex = 0u, erasedIndex = 0u; parentIndex != result.size(); ++parentIndex) {

				const auto* const parent = result[parentIndex].get();
				for (auto* component : _components) {
					if (component->getParentComponent() == parent) {
						result.emplace_back(VolatileComponent<> { ::hg::move(component) });
					}
				}

				while (erasedIndex != result.size()) {
					_components.erase(result[erasedIndex].get());
					++erasedIndex;
				}
			}

			/**/

			if (_universe.has_value()) {
				for (const auto& removal : result) {
					scheduleComponentRemoval(removal.get());
				}
			}

			return result;
		}

		template <IsConstructibleComponent Component_>
		[[nodiscard]] AutoArray<VolatileComponent<Component_>> removeComponents() {
			::hg::todo_panic();

			#if 0
			const auto removal = selectComponents(
				[](ptr<const HierarchyComponent> component_) {
					return IsType < Component_ > (*component_);
				}
			);

			/**/

			auto result = AutoArray<VolatileComponent<Component_>> {};
			result.reserve(removal.size());

			for (auto* const component : removal) {
				result.emplace_back(
					removeComponent < Component_ > (static_cast<ref<Component_>>(*component))
				);
			}

			/**/

			return result;
			#endif
		}

		template <IsConstructibleComponent Component_>
		void destroyComponent(ref<Component_> component_) {
			auto* const address = std::addressof(component_);
			_components.erase(address);

			/**/

			for (auto measured = _components.size() - 1; measured != _components.size(); measured = _components.size()) {
				const auto nextIt = std::ranges::find(
					_components.values(),
					address,
					[](const auto* const component_) { return component_->getParentComponent(); }
				);

				if (nextIt != _components.end()) {
					destroyComponent(**nextIt);
				}
			}

			/**/

			if (_universe.has_value()) {
				scheduleComponentRemoval(clone(address));
			}

			scheduleComponentRelease(address);
		}

	public:
		/**
		 * Callback to register all related components of this actor
		 *
		 * @author Julius
		 * @date 01.12.2021
		 *
		 * @param universe_ The universe where is actor will be committed to.
		 * @param context_ The context where to register components.
		 */
		void registerComponents(_In_ ref<engine::core::Universe> universe_, _Inout_ ref<IComponentRegisterContext> context_);

		void unregisterComponents(_Inout_ ref<IComponentRegisterContext> context_);
	};

	/**/

	template <class ActorType_ = Actor>
	class VolatileActor final :
		public UniquePtr<ActorType_, void(*)(ptr<Actor>)> {
	public:
		template <class Type_>
		friend class VolatileActor;

	public:
		using this_type = VolatileActor<ActorType_>;
		using actor_type = ActorType_;
		using base_type = UniquePtr<ActorType_, void(*)(ptr<Actor>)>;

	public:
		using base_type::base_type;

		VolatileActor() = delete;

		constexpr explicit VolatileActor(_Inout_ mref<ptr<ActorType_>> actor_) noexcept :
			base_type(actor_, VolatileActor<Actor>::destroy) {}

		template <class Type_>
		constexpr explicit VolatileActor(_Inout_ mref<VolatileActor<Type_>> other_) noexcept :
			// TODO: Replace with secure down cast
			base_type(static_cast<typename base_type::pointer>(other_.release()), VolatileActor<Actor>::destroy) {}

	public:
		static void destroy(ptr<Actor> obj_);
	};

	/**/

	/**
	 * Registers a new ActorClass to the engine
	 *
	 * @author Julius
	 * @date 25.11.2021
	 *
	 * @param class_ The reflected actor class instance to register.
	 *
	 * @returns True if register succeeded, otherwise false
	 */
	extern bool RegisterActorClass(_In_ ptr<const ActorClass> class_) noexcept;

	/**
	 * Registers a new ActorType to the engine
	 *
	 * @author Julius
	 * @date 25.11.2021
	 *
	 * @details This template invocation will automatically generate a reflected ActorClass to register to the engine
	 *
	 * @tparam ActorType_ The actor class type to register
	 *
	 * @returns True if register succeeded, otherwise false
	 */
	template <class ActorType_>
	bool RegisterActorClass() noexcept {
		return RegisterActorClass(Reflect::SubstitudeActorClass<ActorType_>::Unknown());
	}

	/**
	 * Create a new default actor object
	 *
	 * @details This will not populate the scene like calling a actor spawn function.
	 *
	 * @author Julius
	 * @date 25.11.2021
	 *
	 * @returns A pointer to the newly created actor if succeeded, otherwise nullptr
	 */
	[[nodiscard]] extern VolatileActor<> CreateActor();

	/**
	 * Create a new default actor object
	 *
	 * @details This will not populate the scene like calling a actor spawn function.
	 *
	 * @author Julius
	 * @date 25.11.2021
	 *
	 * @returns A future, containing the newly created actor if succeeded, otherwise nullptr
	 */
	[[nodiscard]] extern Future<VolatileActor<>> CreateActor(async_t);

	/**
	 * Creates a new actor object based on given actor class
	 *
	 * @details This will not populate the scene like calling a actor spawn function.
	 *
	 * @author Julius
	 * @date 25.11.2021
	 *
	 * @param class_ The actor class to instantiate
	 *
	 * @returns A pointer to the newly created actor if succeeded, otherwise nullptr
	 */
	[[nodiscard]] extern VolatileActor<> CreateActor(_In_ ptr<const ActorClass> class_) noexcept;

	template <class ActorType_>
	[[nodiscard]] VolatileActor<ActorType_> CreateActor() {
		// TODO: Secure cast
		return static_cast<VolatileActor<ActorType_>>(CreateActor(Reflect::SubstitudeActorClass<ActorType_>::Known()));
	}

	/**
	 * Create a new actor object based on given actor class
	 *
	 * @details This will not populate the scene like calling a actor spawn function.
	 *
	 * @author Julius
	 * @date 26.11.2021
	 *
	 * @param class_ The actor class to instantiate.
	 * @param serialized_ The serialized data to use.
	 *
	 * @returns A future, containing the newly created actor is succeeded, otherwise nullptr
	 */
	[[nodiscard]] extern Future<VolatileActor<>> CreateActor(
		_In_ ptr<const ActorClass> class_,
		_In_ cref<SerializedActor> serialized_
	) noexcept;

	/**
	 * Creates a new actor object equivalent to given actor object
	 *
	 * @details This will not populate the scene like calling a actor spawn function.
	 *
	 * @author Julius
	 * @date 25.11.2021
	 *
	 * @param actor_ The actor to clone from.
	 * @param universe_ The universe where to create the new actor
	 *
	 * @returns A future, containing the newly created actor if succeeded, otherwise nullptr
	 */
	[[nodiscard]] extern Future<ptr<Actor>> CloneActorInto(
		_In_ cref<Actor> actor_,
		_In_ cref<Universe> universe_
	) noexcept;

	/**
	 * Creates a new actor object based on given actor class and mounts it
	 *
	 * @author Julius
	 * @date 25.11.2021
	 *
	 * @param class_ The actor class to instantiate.
	 * @param universe_ The universe where to create and place the new actor
	 *
	 * @returns A future, containing the newly created and mounted actor if succeeded, otherwise nullptr
	 */
	[[nodiscard]] extern Future<ptr<Actor>> SpawnActor(
		_In_ ptr<const ActorClass> class_,
		_In_ cref<Universe> universe_
	) noexcept;

	/**
	 * Creates a new actor object based on given actor class and mounts it
	 *
	 * @author Julius
	 * @date 25.11.2021
	 *
	 * @details Will populate the instantiated actor by given serialized data
	 *
	 * @param class_ The actor class to instantiate.
	 * @param serialized_ The serialized data to use.
	 * @param universe_ The universe where to create and place the new actor
	 *
	 * @returns A future, containing the newly created and mounted actor if succeeded, otherwise nullptr
	 */
	[[nodiscard]] extern Future<ptr<Actor>> SpawnActor(
		_In_ ptr<const ActorClass> class_,
		_In_ cref<SerializedActor> serialized_,
		_In_ cref<Universe> universe_
	) noexcept;

	/**
	 * Destroys the provided volatile actor instance
	 *
	 * @param actor_ The volatile actor to destroy.
	 *
	 * @returns True if succeeded, otherwise false.
	 */
	[[nodiscard]] extern bool Destroy(mref<VolatileActor<>> actor_) noexcept;

	/**
	 * Destroys the given actor
	 *
	 * @author Julius
	 * @date 25.11.2021
	 *
	 * @param actor_ The actor to destroy.
	 * @param universe_ The universe where to destroy the actor.
	 *
	 * @returns A future, representing whether the actor was successfully destroyed.
	 */
	[[nodiscard]] extern Future<bool> Destroy(
		mref<ptr<Actor>> actor_,
		cref<Universe> universe_
	) noexcept;
}