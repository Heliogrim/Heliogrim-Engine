#pragma once

#include <Engine.ACS/Registry.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.Serialization/Structure/IntegralScopedSlot.hpp>
#include <Engine.Serialization/Structure/RecordScopedSlot.hpp>
#include <Engine.Serialization/Structure/SeqScopedSlot.hpp>
#include <Engine.Serialization/Structure/StructScopedSlot.hpp>
#include <Engine.Serialization.Structures/Symbols.hpp>
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
				ActorInitializer { *allocator_.get() },
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

			ActorGuid actorGuid {};
			access::Structure<Guid>::hydrate(slot.getStructSlot(Symbols::Guid), actorGuid);

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
			auto instanced = Vector<nmpt<LogicComponent>>();
			instanced.resize(count, nullptr);

			for (s64 idx = 0; idx < count; ++idx) {

				auto record = components.getRecordSlot(idx);
				auto structured = record.asStruct();

				/**/

				type_id checkTypeId {};
				structured.getSlot<u64>(Symbols::Type) >> checkTypeId.data;
				::hg::assertrt(checkTypeId != type_id {});

				/**/

				s8 recordParentIdx { -1 };
				structured.getSlot<s8>(Symbols::Parent) >> recordParentIdx;
				::hg::assertrt(recordParentIdx == -1 || recordParentIdx >= 0);

				/**/

				const auto fnSet = serial->getComponentScheme(checkTypeId);
				::hg::assertrt(fnSet.has_value());

				instanced[idx] = fnSet->deserialize(
					record,
					allocator_,
					*target,
					recordParentIdx >= 0 ?
					// Error: Unsafe / UB
					static_cast<ptr<HierarchyComponent>>(instanced[recordParentIdx].get()) :
					nullptr
				);
				::hg::assertrt(instanced[idx] != nullptr);

				// Error: Unsafe / UB
				target->addComponent(static_cast<ptr<HierarchyComponent>>(instanced[idx].get()));
			}

			/**/

			return target;
		}
	};
}
