#pragma once

#include <Engine.ACS/Registry.hpp>
#include <Engine.ACS/Actor/ActorGuid.hpp>

#include "SelectScheme.hpp"
#include "SerializationModule.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
import Engine.Core;
import Engine.Reflect;
import Engine.Serialization;
import Engine.Serialization.Structures;
#endif

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

			using result_type = decltype(AccessType_::template instantiate(
				allocator_,
				std::declval<ref<const ActorGuid>>(),
				std::forward<decltype(args_)>(args_)...
			));
			const auto original = slot_.asStruct();
			if (original.getRecordSlot(Symbols::Components).asSeq().getRecordCount() < 0LL) {
				// getRecordCount already latched the failure on the context; create no actor.
				return static_cast<result_type>(nullptr);
			}

			const auto serial = Engine::getEngine()->getSerialization();
			const auto& migrations = serial->migrations();
			const auto& root = original.slot()->getState().root;
			auto ctx = root != nullptr ? root->context : nullptr;
			const auto migrate = [&ctx, &migrations](ref<const StructScopedSlot> record_) -> Opt<Migrated> {
				// shortcut: a contextless load hydrates as written, without migration; give every load a context to upgrade
				if (ctx == nullptr) {
					return SomeVal<Migrated>(StructScopedSlot { record_ });
				}
				return migrations.migrateToCurrent(get_typeId_slot(record_), record_, *ctx);
			};

			const auto migrated = migrate(original);
			if (not migrated.has_value()) {
				return static_cast<result_type>(nullptr);
			}

			const auto& slot = migrated->slot();
			std::ignore = get_typeId_slot(slot);
			const auto actorGuid = get_guid_slot<ActorGuid>(slot);

			auto components = slot.getRecordSlot(Symbols::Components).asSeq();
			const auto count = components.getRecordCount();
			if (count < 0LL) {
				// getRecordCount already latched the failure on the context; create no actor.
				return static_cast<result_type>(nullptr);
			}

			// An actor missing one of its components cannot round-trip; skip it whole. A level skips with a warning, a root actor has nothing left to load.
			const bool isRoot = original.slot()->getState().parent == nullptr;
			const auto skip = [isRoot](ref<const StructScopedSlot> structured_, ref<const String> detail_) {
				if (isRoot) {
					structured_.slot()->fail(ReadError::Kind::eUnknownType, structured_.slot()->getState().offset, 0LL, detail_.c_str());
				} else {
					structured_.warn(ReadError::Kind::eUnknownType, detail_);
				}
				return static_cast<result_type>(nullptr);
			};
			for (s64 idx = 0; idx < count; ++idx) {
				const auto structured = components.getRecordSlot(idx).asStruct();
				if (not has_typeId_slot(structured)) {
					return skip(structured, std::format("component '{}' has no '@type'; actor skipped", idx));
				}

				if (not serial->getComponentScheme(get_typeId_slot<ComponentTypeId>(structured)).has_value()) {
					return skip(structured, std::format("component '{}' type has no scheme; actor skipped", idx));
				}

				s8 parentIdx { -1 };
				structured.template getSlot<s8>(Symbols::Parent) >> parentIdx;
				if (parentIdx < -1 || parentIdx >= idx) {
					const auto detail = std::format(
						"parent index '{}' is out of range for component '{}', expected -1 to {}",
						parentIdx,
						idx,
						idx - 1
					);
					structured.slot()->fail(ReadError::Kind::eInvalidValue, structured.slot()->getState().offset, 0LL, detail.c_str());
					return static_cast<result_type>(nullptr);
				}
			}

			const auto warned = ctx != nullptr ? ctx->warningCount() : 0uz;
			auto* target = AccessType_::template instantiate(
				allocator_,
				actorGuid,
				std::forward<decltype(args_)>(args_)...
			);
			AccessType_::hydrate(slot, *target);
			if (ctx != nullptr && ctx->failed()) {
				// shortcut: the half-hydrated actor leaks (known default-deleter crash); release it once that is fixed
				return static_cast<result_type>(nullptr);
			}

			/**/

			bool unknown = false;
			auto instanced = Vector<ptr<LogicComponent>>();
			instanced.resize(count, nullptr);

			for (s64 idx = 0; idx < count; ++idx) {

				const auto component = migrate(components.getRecordSlot(idx).asStruct());
				if (not component.has_value()) {
					unknown = true;
					continue;
				}

				const auto& structured = component->slot();
				const auto record = RecordScopedSlot {
					ScopedSlotState { structured.getScopedState() },
					StructureSlotState { structured.slot()->getState() }
				};

				/**/

				const auto checkTypeId = get_typeId_slot<ComponentTypeId>(structured);
				s8 recordParentIdx { -1 };
				structured.template getSlot<s8>(Symbols::Parent) >> recordParentIdx;

				const auto fnSet = serial->getComponentScheme(checkTypeId);
				if (not fnSet.has_value()) {
					// Unreachable after the pre-pass; kept so a scheme race cannot crash the load.
					unknown = true;
					continue;
				}

				const auto hasParent = recordParentIdx >= 0 && recordParentIdx < idx;
				auto* const parent = hasParent && instanced[recordParentIdx] ?
					Cast<HierarchyComponent>(instanced[recordParentIdx]) :
					nullptr;
				if (hasParent && parent == nullptr) {
					// The parent failed or is no hierarchy component; the child cannot attach.
					unknown = true;
					continue;
				}

				/**/

				auto next = fnSet->deserialize(
					record,
					allocator_,
					*target,
					parent
				);

				unknown |= structured.reportUnreadKeys();

				/**/

				if (next.get() == nullptr) {
					unknown = true;
					continue;
				}

				if (IsType<HierarchyComponent>(*next.get()) && parent == nullptr && target->getRootComponent().has_value()) {
					// addComponent asserts a single root; a second parentless hierarchy component is bad data.
					structured.slot()->fail(
						ReadError::Kind::eInvalidValue,
						structured.slot()->getState().offset,
						0LL,
						"actor has more than one root component"
					);
					unknown = true;

				} else if (IsType<HierarchyComponent>(*next.get())) {
					// Keep back-reference for parent resolving with pure pointers
					instanced[idx] = next.get();
					target->addComponent(
						VolatileComponent<HierarchyComponent> { ::hg::move(next) },
						Some<ref<const typename Type_::AddComponentOptions>>(
							{
								.autoRegister = false,
								.parent = !!parent ? Some<ref<HierarchyComponent>>(*parent) : None
							}
						)
					);

				} else {
					// Actors only attach hierarchy components; the dropped one must not vanish silently on save.
					unknown = true;
				}
			}

			/**/

			// A warned (e.g. unresolved) value is dropped from the object, so a later save must be refused.
			if (slot.reportUnreadKeys() || unknown || (ctx != nullptr && ctx->warningCount() != warned)) {
				target->markUnknownData();
			}
			return target;
		}
	};
}
