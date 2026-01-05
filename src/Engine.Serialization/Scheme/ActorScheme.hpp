#pragma once

#include <Engine.ACS/Registry.hpp>
#include <Engine.Common/Optional.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.Reflect/Cast.hpp>
#include <Engine.Serialization/Structure/IntegralScopedSlot.hpp>
#include <Engine.Serialization/Structure/RecordScopedSlot.hpp>
#include <Engine.Serialization/Structure/SeqScopedSlot.hpp>
#include <Engine.Serialization/Structure/StructScopedSlot.hpp>
#include <Engine.Serialization.Structures/Symbols.hpp>
#include <Engine.Serialization.Structures/Common/Guid.hpp>
#include <Engine.Serialization.Structures/Common/TypeId.hpp>
#include <Heliogrim/Actor/ActorGuid.hpp>

#include "Scheme.hpp"
#include "__fwd.hpp"

namespace hg::engine::serialization {
	/**/

	template <CompleteType Type_> requires std::derived_from<Type_, Actor>
	struct ActorScheme : public Scheme<Type_> {
		template <typename>
		[[nodiscard]] static decltype(auto) instantiate(
			nmpt<acs::Registry> allocator_,
			_In_ cref<ActorGuid> guid_,
			auto&&... args_
		) {
			return allocator_->createActor<Type_>(
				guid_,
				ActorInitializer { *allocator_.get(), { .autoInitialize = false } },
				std::forward<decltype(args_)>(args_)...
			);
		}

		template <typename AccessType_>
		[[nodiscard]] static decltype(auto) deserialize(
			_In_ cref<RecordScopedSlot> slot_,
			nmpt<acs::Registry> allocator_,
			auto&&... args_
		) {

			const auto slot = slot_.asStruct();
			const auto actorGuid = get_guid_slot<ActorGuid>(slot);

			auto* target = AccessType_::template instantiate(
				allocator_,
				actorGuid,
				std::forward<decltype(args_)>(args_)...
			);
			AccessType_::hydrate(slot, *target);

			/**/

			const auto serial = Engine::getEngine()->getSerialization();
			// Error: SEH / We need to check whether there is any component data available
			auto components = slot.getRecordSlot(Symbols::Components).asSeq();

			// TODO:
			const auto count = components.getRecordCount();
			auto instanced = Vector<ptr<LogicComponent>>();
			instanced.resize(count, nullptr);

			for (s64 idx = 0; idx < count; ++idx) {

				auto record = components.getRecordSlot(idx);
				auto structured = record.asStruct();

				/**/

				const auto checkTypeId = get_typeId_slot<ComponentTypeId>(structured);
				::hg::assertrt(checkTypeId != ComponentTypeId {});

				/**/

				s8 recordParentIdx { -1 };
				structured.getSlot<s8>(Symbols::Parent) >> recordParentIdx;
				::hg::assertrt(recordParentIdx == -1 || (recordParentIdx >= 0 && recordParentIdx < /* ~instanced.size() */idx));

				/**/

				const auto fnSet = serial->getComponentScheme(checkTypeId);
				::hg::assertrt(fnSet.has_value());

				const auto hasParent = recordParentIdx >= 0;
				auto* const parent = hasParent && instanced[recordParentIdx] ?
					Cast<HierarchyComponent>(instanced[recordParentIdx]) :
					nullptr;
				::hg::assertd(hasParent == !!parent);

				/**/

				auto next = fnSet->deserialize(
					record,
					allocator_,
					*target,
					parent
				);

				/**/

				if (IsType<HierarchyComponent>(*next.get())) {
					// Keep back-reference for parent resolving with pure pointers
					instanced[idx] = next.get();
					target->addComponent(
						VolatileComponent<HierarchyComponent> { ::hg::move(next) },
						Some<ref<const Type_::AddComponentOptions>>(
							{
								.autoRegister = false,
								.parent = !!parent ? Some<ref<HierarchyComponent>>(*parent) : None
							}
						)
					);
				}
			}

			/**/

			return target;
		}
	};
}
