#pragma once

#include <span>
#include <Engine.ACS/Archetype/LayoutInfo.hpp>
#include <Engine.ACS/Cluster/ClusterRegion.hpp>
#include <Engine.Common/Types.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Reflect/Meta/MetaClass.hpp>

#include "ArchetypeOps.hpp"

namespace hg {
	/**
	 * Forward Declaration
	 */
	class Actor;
	class HierarchyComponent;

	template <typename>
	class TypedActorClass;

	/**
	 * Everything captured from an archetype's templates: meta class, slot layout and operations.
	 *
	 * Exists only for registered archetypes; built once per archetype by `make<T>()` as a `constinit` static
	 * (see `ArchetypeDefinition.inl.hpp`). Not copyable, compared by address.
	 */
	class ActorClass {
	public:
		template <typename>
		friend class ::hg::TypedActorClass;

	private:
		constexpr ActorClass(
			const ptr<const MetaClass> metaClass_,
			const u64 layoutHash_,
			const engine::acs::ClusterDesc desc_,
			ref<const ArchetypeOps> ops_,
			const std::span<const uintptr_t> componentOffsets_,
			const ptr<const u64> startBitmap_,
			const u32 minAlign_,
			const std::span<const engine::acs::SlotEntry> entries_,
			const std::span<const std::string_view> slotPaths_
		) noexcept :
			_metaClass(metaClass_),
			_layoutHash(layoutHash_),
			_desc(desc_),
			_ops(ops_),
			_componentOffsets(componentOffsets_),
			_startBitmap(startBitmap_),
			_minAlign(minAlign_),
			_entries(entries_),
			_slotPaths(slotPaths_) {}

	public:
		ActorClass(const ActorClass&) = delete;

		ActorClass& operator=(const ActorClass&) = delete;

		constexpr ~ActorClass() noexcept = default;

	public:
		/**
		 * Builds the class of an archetype; defined in `ArchetypeDefinition.inl.hpp`.
		 */
		template <typename Type_>
		[[nodiscard]] constexpr static TypedActorClass<Type_> make() noexcept;

	private:
		ptr<const MetaClass> _metaClass;
		u64 _layoutHash;
		engine::acs::ClusterDesc _desc;
		ArchetypeOps _ops;
		std::span<const uintptr_t> _componentOffsets;
		ptr<const u64> _startBitmap;
		u32 _minAlign;
		std::span<const engine::acs::SlotEntry> _entries;
		std::span<const std::string_view> _slotPaths;

	public:
		[[nodiscard]] constexpr ptr<const MetaClass> metaClass() const noexcept {
			return _metaClass;
		}

		[[nodiscard]] constexpr u64 layoutHash() const noexcept {
			return _layoutHash;
		}

		[[nodiscard]] constexpr engine::acs::ClusterDesc desc() const noexcept {
			return _desc;
		}

		[[nodiscard]] constexpr cref<ArchetypeOps> ops() const noexcept {
			return _ops;
		}

		/**
		 * Offset of each component's `HierarchyComponent` subobject from the slot base, in entry order
		 * (parents before children).
		 */
		[[nodiscard]] constexpr std::span<const uintptr_t> componentOffsets() const noexcept {
			return _componentOffsets;
		}

		/**
		 * One bit per `minAlign()` step of the component block, set at each component start.
		 */
		[[nodiscard]] constexpr ptr<const u64> startBitmap() const noexcept {
			return _startBitmap;
		}

		[[nodiscard]] constexpr u32 minAlign() const noexcept {
			return _minAlign;
		}

		/**
		 * Layout entries (offset from slot base, size, parent entry, array index) in entry order.
		 */
		[[nodiscard]] constexpr std::span<const engine::acs::SlotEntry> entries() const noexcept {
			return _entries;
		}

		/**
		 * Slot path of each entry (without element index, see `entries()[i].arrayIndex`); for serialization only.
		 */
		[[nodiscard]] constexpr std::span<const std::string_view> slotPaths() const noexcept {
			return _slotPaths;
		}

	private:
		/* Relocation helpers; members of the friend-granted class so the templates in `make` can reach private links. */

		static void rebind(ref<HierarchyComponent> component_, ptr<Actor> owner_, ptr<HierarchyComponent> parent_) noexcept;

		static void rebindRoot(ref<Actor> actor_, ref<HierarchyComponent> root_) noexcept;

		// Move-constructs the actor members that are not byte-safe (label, component set) over their byte copies in `dst_`
		// and destroys them in `src_`.
		static void reseat(ref<Actor> dst_, ref<Actor> src_) noexcept;
	};
}
