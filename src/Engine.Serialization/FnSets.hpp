#pragma once

#include "Access/Structure.hpp"
#include "Engine.Common/Optional.hpp"
#include "Engine.Common/Collection/DenseMap.hpp"
#include "Engine.Common/Functional/Function.hpp"
#include "Engine.Common/Meta/TypeId.hpp"
#include "Engine.Reflect/IsType.hpp"
#include "Heliogrim/ActorInitializer.hpp"
#include "Heliogrim/Actor/ActorGuid.hpp"
#include "Structure/RecordScopedSlot.hpp"

namespace hg::engine::serialization {
	struct ActorFnSet {
		std::function<void(cref<Actor>, mref<StructScopedSlot>)> serialize;

		std::function<ptr<Actor>(
			cref<RecordScopedSlot>,
			nmpt<acs::Registry>
		)> deserialize;
	};

	struct ComponentFnSet {
		std::function<void(cref<LogicComponent>, mref<StructScopedSlot>)> serialize;

		std::function<ptr<LogicComponent>(
			cref<RecordScopedSlot>,
			nmpt<acs::Registry>,
			ref<Actor>,
			nmpt<HierarchyComponent>
		)> deserialize;
	};
}

namespace hg::engine::serialization {
	class FnSets {
	public:
		using this_type = FnSets;

	protected:
		DenseMap<type_id, serialization::ActorFnSet> _actorStructuredSchemes;
		DenseMap<type_id, serialization::ComponentFnSet> _componentStructuredSchemes;

	public:
		template <class ActorType_>
		void addActorScheme() {
			using record_type = serialization::RecordScopedSlot;
			using struct_type = serialization::StructScopedSlot;

			constexpr type_id typeId = ActorType_::typeId;
			_actorStructuredSchemes.insert_or_assign(
				typeId,
				serialization::ActorFnSet {
					.serialize = [](cref<Actor> actor_, mref<struct_type> slot_) -> void {
						::hg::assertd(IsType<ActorType_>(actor_));
						serialization::access::Structure<ActorType_>::serialize(
							static_cast<cref<ActorType_>>(actor_),
							std::move(slot_)
						);
					},
					.deserialize = [](cref<record_type> slot_, nmpt<acs::Registry> allocator_) -> ptr<Actor> {
						return serialization::access::Structure<ActorType_>::template deserialize(slot_, allocator_);
					}
				}
			);
		}

		[[nodiscard]] decltype(auto) getActorScheme(const type_id typeId_) const noexcept {
			const auto it = _actorStructuredSchemes.find(typeId_);
			return it != _actorStructuredSchemes.cend() ? Some(it->second) : None;
		}

		template <class ActorType_>
		[[nodiscard]] decltype(auto) getActorScheme() const noexcept {
			constexpr type_id typeId = ActorType_::typeId;
			return getActorScheme(typeId);
		}

		template <class ComponentType_>
		void addComponentScheme() {
			using record_type = serialization::RecordScopedSlot;
			using struct_type = serialization::StructScopedSlot;

			constexpr type_id typeId = ComponentType_::typeId;
			_componentStructuredSchemes.insert_or_assign(
				typeId,
				serialization::ComponentFnSet {
					.serialize = [](cref<LogicComponent> component_, mref<struct_type> slot_) -> void {
						::hg::assertd(IsType<ComponentType_>(component_));
						serialization::access::Structure<ComponentType_>::serialize(
							static_cast<cref<ComponentType_>>(component_),
							std::move(slot_)
						);
					},
					.deserialize = [](
					cref<record_type> slot_,
					nmpt<acs::Registry> allocator_,
					ref<Actor> owner_,
					nmpt<HierarchyComponent> maybeParent_
				) -> ptr<LogicComponent> {
						return serialization::access::Structure<ComponentType_>::template deserialize(
							slot_,
							allocator_,
							owner_,
							maybeParent_
						);
					}
				}
			);
		}

		[[nodiscard]] decltype(auto) getComponentScheme(const type_id typeId_) const noexcept {
			const auto it = _componentStructuredSchemes.find(typeId_);
			return it != _componentStructuredSchemes.cend() ? Some(it->second) : None;
		}

		template <class ComponentType_>
		[[nodiscard]] decltype(auto) getComponentScheme() const noexcept {
			constexpr type_id typeId = ComponentType_::typeId;
			return getComponentScheme(typeId);
		}
	};
}
