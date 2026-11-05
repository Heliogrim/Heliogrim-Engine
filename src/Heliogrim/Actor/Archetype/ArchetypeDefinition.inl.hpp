#pragma once

/**
 * Heavy archetype instantiation: include only in the one `.cpp` that defines an archetype, followed by
 * `template class InheritArchetype<Derived, Base>;`.
 */

#include <algorithm>
#include <memory>
#include <new>
#include <utility>
#include <Engine.ACS/ComponentGuid.hpp>
#include <Engine.Asserts/Asserts.hpp>
#include <Engine.Common/Move.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/Array.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/Meta/Relocatable.hpp>
#include <Engine.Serialization.Structures/Symbols.hpp>

#include "ActorClass.hpp"
#include "ArchetypeConcept.hpp"
#include "InheritArchetype.hpp"
#include "TypedActorClass.hpp"
#include "../Actor.hpp"
#include "../../ActorInitializer.hpp"
#include "../../Component/HierarchyComponent.hpp"
// After InheritArchetype.hpp: both use `slot_type_t`.
#include "ArchetypeLayout.inl.hpp"
#include "ArchetypeStructured.inl.hpp"

namespace hg::archetype_detail {
	template <typename Type_>
	using info_of = engine::acs::LayoutInfo<typename Type_::layout, Type_>;

	template <typename Info_, u32 Entry_>
	[[nodiscard]] auto* component_at(std::byte* const slot_) noexcept {
		using type = slot_type_t<Info_, Info_::slot_of(Entry_)>;
		return std::launder(reinterpret_cast<type*>(slot_ + Info_::entries[Entry_].offset));
	}

	template <typename Info_, u32 Entry_>
	[[nodiscard]] ptr<HierarchyComponent> parent_at(std::byte* const slot_) noexcept {
		constexpr u32 parent = Info_::entries[Entry_].parentIndex;
		if constexpr (parent == ~0u) {
			return nullptr;
		} else {
			return component_at<Info_, parent>(slot_);
		}
	}

	template <typename Info_, u32 Entry_>
	void construct_entry(std::byte* const slot_, const ActorGuid guid_, const ptr<Actor> actor_) {
		using type = slot_type_t<Info_, Info_::slot_of(Entry_)>;
		static_assert(std::derived_from<type, HierarchyComponent>, "Archetype slots hold HierarchyComponent types.");

		auto* const component = ::new(static_cast<void*>(slot_ + Info_::entries[Entry_].offset)) type(
			generate_component_guid(),
			CachedActorPointer { guid_, actor_ },
			parent_at<Info_, Entry_>(slot_)
		);

		// shortcut: offset-0 single inheritance assumption. componentOffsets point at the HierarchyComponent subobject
		// only while it sits at offset 0, and construct converts the actor storage to Actor* before the actor's
		// lifetime starts (the components only store it); upgrade with per-type base offsets and a proper cast path
		// once a multiple-inheritance component or a non-zero Actor base offset appears.
		::hg::assertd(static_cast<const void*>(static_cast<ptr<HierarchyComponent>>(component)) == component);
	}

	template <typename Info_, u32... Entries_>
	[[nodiscard]] consteval bool all_relocatable(std::integer_sequence<u32, Entries_...>) {
		return (relocatable_v<slot_type_t<Info_, Info_::slot_of(Entries_)>> && ...);
	}

	template <typename Info_, u32 Entry_>
	void move_entry(std::byte* const src_, std::byte* const dst_) {
		using type = slot_type_t<Info_, Info_::slot_of(Entry_)>;
		constexpr u32 offset = Info_::entries[Entry_].offset;

		if constexpr (relocatable_v<type>) {
			std::memcpy(dst_ + offset, src_ + offset, sizeof(type));
		} else {
			auto* const from = component_at<Info_, Entry_>(src_);
			::new(static_cast<void*>(dst_ + offset)) type(::hg::move(*from));
			std::destroy_at(from);
		}
	}

	template <typename Type_>
	[[nodiscard]] constexpr u64 layout_hash() noexcept {
		using info = info_of<Type_>;

		// FNV-1a over the slot table.
		u64 hash = 0xCBF29CE484222325uLL;
		const auto mix = [&hash](const u64 value_) {
			for (u32 byte = 0u; byte < 8u; ++byte) {
				hash ^= (value_ >> (byte * 8u)) & 0xFFuLL;
				hash *= 0x100000001B3uLL;
			}
		};

		mix(Type_::typeId.data);
		mix(info::slot_stride);
		mix(info::actor_offset);
		mix(sizeof(Type_));
		for (const auto& entry : info::entries) {
			mix(entry.offset);
			mix(entry.size);
			mix(entry.parentIndex);
			mix(entry.arrayIndex);
		}
		// Stable TypeIds, not the layout-local typeIndex.
		[&]<u32... Entries_>(std::integer_sequence<u32, Entries_...>) {
			((
				mix(slot_type_t<info, info::slot_of(Entries_)>::typeId.data),
				mix(layout_version<slot_type_t<info, info::slot_of(Entries_)>>())
			), ...);
		}(std::make_integer_sequence<u32, info::entry_count> {});
		return hash;
	}

	template <typename Type_>
	struct ClassTables {
		using info = info_of<Type_>;

		constexpr static Array<uintptr_t, info::entry_count> offsets = [] {
			Array<uintptr_t, info::entry_count> result {};
			for (u32 entry = 0u; entry < info::entry_count; ++entry) {
				result[entry] = info::entries[entry].offset;
			}
			return result;
		}();

		constexpr static Array<ArchetypeEntryType, info::entry_count> types =
			[]<u32... Entries_>(std::integer_sequence<u32, Entries_...>) {
				return Array<ArchetypeEntryType, info::entry_count> {
					ArchetypeEntryType {
						&engine::acs::detail::type_tag<slot_type_t<info, info::slot_of(Entries_)>>,
						static_cast<ptr<const MetaClass>>(slot_type_t<info, info::slot_of(Entries_)>::getStaticMetaClass())
					}...
				};
			}(std::make_integer_sequence<u32, info::entry_count> {});

		[[nodiscard]] constexpr static ptr<const u64> bitmap() noexcept {
			if constexpr (info::bitmap_words == 1u) {
				return &info::startBitmap;
			} else {
				return info::startBitmap.data();
			}
		}
	};
}

namespace hg {
	template <typename Type_>
	constexpr TypedActorClass<Type_> ActorClass::make() noexcept {
		using info = archetype_detail::info_of<Type_>;
		using tables = archetype_detail::ClassTables<Type_>;
		using entry_sequence = std::make_integer_sequence<u32, info::entry_count>;

		static_assert(
			relocatable_v<Type_>,
			"Archetype actors are relocated by byte copy, with Actor's label and component set re-seated by move."
		);

		ArchetypeOps ops {};

		ops.construct = [](std::byte* const slots_, const u32 count_, const u32 stride_, cref<ArchetypeInit> init_) {
			for (u32 index = 0u; index < count_; ++index) {
				std::byte* const slot = slots_ + index * stride_;
				auto* const storage = slot + info::actor_offset;
				// See the shortcut in construct_entry: converted to Actor* before the actor's lifetime starts.
				const ptr<Actor> actor = reinterpret_cast<Type_*>(storage);
				const auto guid = generate_actor_guid();

				[&]<u32... Entries_>(std::integer_sequence<u32, Entries_...>) {
					(archetype_detail::construct_entry<info, Entries_>(slot, guid, actor), ...);
				}(entry_sequence {});

				auto initializer = ActorInitializer { init_.registry };
				initializer._guid = guid;
				initializer._archetype = &Type_::actorClass();
				::new(static_cast<void*>(storage)) Type_(initializer);
			}
		};

		ops.destroy = [](std::byte* const slots_, const u32 count_, const u32 stride_) {
			constexpr auto actorAt = [](std::byte* const slot_) noexcept {
				return std::launder(reinterpret_cast<Type_*>(slot_ + info::actor_offset));
			};

			for (u32 index = 0u; index < count_; ++index) {
				std::byte* const slot = slots_ + index * stride_;
				std::destroy_at(actorAt(slot));

				// Children before parents.
				[&]<u32... Entries_>(std::integer_sequence<u32, Entries_...>) {
					(std::destroy_at(archetype_detail::component_at<info, info::entry_count - 1u - Entries_>(slot)), ...);
				}(entry_sequence {});
			}
		};

		ops.relocate = [](std::byte* const src_, std::byte* const dst_, const u32 count_, const u32 stride_) {
			constexpr auto actorAt = [](std::byte* const slot_) noexcept {
				return std::launder(reinterpret_cast<Type_*>(slot_ + info::actor_offset));
			};

			if constexpr (archetype_detail::all_relocatable<info>(entry_sequence {})) {
				std::memcpy(dst_, src_, static_cast<size_t>(count_) * stride_);
			}

			for (u32 index = 0u; index < count_; ++index) {
				std::byte* const src = src_ + index * stride_;
				std::byte* const dst = dst_ + index * stride_;

				if constexpr (not archetype_detail::all_relocatable<info>(entry_sequence {})) {
					[&]<u32... Entries_>(std::integer_sequence<u32, Entries_...>) {
						(archetype_detail::move_entry<info, Entries_>(src, dst), ...);
					}(entry_sequence {});
					std::memcpy(dst + info::actor_offset, src + info::actor_offset, sizeof(Type_));
				}

				auto* const actor = actorAt(dst);
				reseat(*actor, *actorAt(src));

				// Links are fixed by the layout, so they are re-stamped instead of offset-adjusted.
				[&]<u32... Entries_>(std::integer_sequence<u32, Entries_...>) {
					(rebind(
						*archetype_detail::component_at<info, Entries_>(dst),
						actor,
						archetype_detail::parent_at<info, Entries_>(dst)
					), ...);
				}(entry_sequence {});
				rebindRoot(*actor, *archetype_detail::component_at<info, 0u>(dst));
			}
		};

		ops.serialize = [](const ptr<const std::byte> slot_, ref<engine::serialization::StructScopedSlot> record_) {
			auto components = record_.insertRecordSlot(engine::serialization::Symbols::Components).intoStruct();
			[&]<u32... Entries_>(std::integer_sequence<u32, Entries_...>) {
				(archetype_detail::serialize_entry<info, Entries_>(slot_, components), ...);
			}(entry_sequence {});
		};

		ops.hydrate = [](const ptr<std::byte> slot_, ref<const engine::serialization::StructScopedSlot> record_) {
			const auto components = record_.getRecordSlot(engine::serialization::Symbols::Components).asStruct();
			[&]<u32... Entries_>(std::integer_sequence<u32, Entries_...>) {
				(archetype_detail::hydrate_entry<info, Entries_>(slot_, components), ...);
			}(entry_sequence {});
		};

		ops.dumpLayout = [](const ptr<const std::byte> slots_, const u32 count_, const u32 stride_, ref<Vector<std::byte>> out_) {
			archetype_detail::dump_layout<info>(archetype_detail::layout_hash<Type_>(), slots_, count_, stride_, out_);
		};

		ops.loadLayout = [](const ptr<std::byte> slots_, const u32 count_, const u32 stride_, std::span<const std::byte> in_) {
			return archetype_detail::load_layout<info>(archetype_detail::layout_hash<Type_>(), slots_, count_, stride_, in_);
		};

		if constexpr (has_tick_v<Type_>) {
			ops.tick = [](std::byte* const slots_, const u32 count_, const u32 stride_) {
				for (u32 index = 0u; index < count_; ++index) {
					std::launder(reinterpret_cast<Type_*>(slots_ + index * stride_ + info::actor_offset))->tick();
				}
			};
		}

		return TypedActorClass<Type_> {
			static_cast<ptr<const MetaClass>>(Type_::getStaticMetaClass()),
			archetype_detail::layout_hash<Type_>(),
			engine::acs::ClusterDesc {
				info::slot_stride,
				std::max(info::block_align, static_cast<u32>(alignof(Type_))),
				info::actor_offset
			},
			ops,
			std::span<const uintptr_t> { tables::offsets },
			tables::bitmap(),
			info::min_align,
			std::span<const engine::acs::SlotEntry> { info::entries },
			std::span<const ArchetypeEntryType> { tables::types }
		};
	}

	template <typename Derived_, typename Base_, typename... Rest_>
	ref<const TypedActorClass<Derived_>> InheritArchetype<Derived_, Base_, Rest_...>::actorClass() noexcept {
		static constinit const TypedActorClass<Derived_> instance = ActorClass::make<Derived_>();
		return instance;
	}
}
