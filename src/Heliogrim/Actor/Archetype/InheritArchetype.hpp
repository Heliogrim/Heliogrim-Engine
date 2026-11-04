#pragma once

#include <cstddef>
#include <memory>
#include <new>
#include <type_traits>
#include <Engine.ACS/Archetype/LayoutInfo.hpp>
#include <Engine.Asserts/Asserts.hpp>
#include <Engine.Common/Types.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/Array.hpp>
#include <Engine.Common/Meta/FixedString.hpp>
#include <Engine.Reflect/Inherit/InheritMeta.hpp>

#include "TypedActorClass.hpp"

namespace hg::archetype_detail {
	template <typename Byte_, typename Type_>
	using like_t = std::conditional_t<std::is_const_v<Byte_>, const Type_, Type_>;

	template <typename Info_, u32 Slot_>
	using slot_type_t = typename Info_::template slot_type<Slot_>;

	template <typename Info_>
	[[nodiscard]] consteval u32 slot_first(const u32 slot_) {
		for (u32 entry = 0u; entry < Info_::entry_count; ++entry) {
			if (Info_::slot_of(entry) == slot_) {
				return entry;
			}
		}
		return ~0u;
	}

	// Entries of a slot over the whole layout (one per element per parent instance).
	template <typename Info_>
	[[nodiscard]] consteval u32 slot_total(const u32 slot_) {
		u32 total = 0u;
		for (u32 entry = 0u; entry < Info_::entry_count; ++entry) {
			total += Info_::slot_of(entry) == slot_ ? 1u : 0u;
		}
		return total;
	}

	// Elements of a slot below one parent instance (its Count).
	template <typename Info_>
	[[nodiscard]] consteval u32 slot_count(const u32 slot_) {
		const u32 first = slot_first<Info_>(slot_);
		u32 count = 0u;
		for (u32 entry = first; entry < Info_::entry_count; ++entry) {
			count += Info_::slot_of(entry) == slot_
				&& Info_::entries[entry].parentIndex == Info_::entries[first].parentIndex ?
				1u :
				0u;
		}
		return count;
	}

	// Entry index distance between consecutive elements below one parent; elements emit their subtrees in a row.
	template <typename Info_>
	[[nodiscard]] consteval u32 element_stride(const u32 slot_) {
		const u32 first = slot_first<Info_>(slot_);
		for (u32 entry = first + 1u; entry < Info_::entry_count; ++entry) {
			if (Info_::slot_of(entry) == slot_ && Info_::entries[entry].parentIndex == Info_::entries[first].parentIndex) {
				return entry - first;
			}
		}
		return 0u;
	}

	template <typename Info_>
	[[nodiscard]] consteval bool descends(u32 entry_, const u32 ancestor_) {
		while (entry_ != ~0u) {
			if (entry_ == ancestor_) {
				return true;
			}
			entry_ = Info_::entries[entry_].parentIndex;
		}
		return false;
	}

	struct ChildMatch {
		u32 matches;
		u32 slot;
	};

	// Slot below `parent_` whose remaining path ends with `Rel_` ("hub", "x/hub").
	template <typename Info_, FixedString Rel_>
	[[nodiscard]] consteval ChildMatch child_slot(const u32 parent_) {
		const auto prefix = Info_::slot_path(parent_);
		ChildMatch result { 0u, ~0u };
		for (u32 slot = 0u; slot < Info_::slot_count; ++slot) {
			const auto path = Info_::slot_path(slot);
			if (path.size() <= prefix.size() + 1uLL || not path.starts_with(prefix) || path[prefix.size()] != '/') {
				continue;
			}
			if (engine::acs::detail::path_matches(path.substr(prefix.size() + 1uLL), Rel_.view())) {
				++result.matches;
				result.slot = slot;
			}
		}
		return result;
	}

	// Per entry of `Parent_`: the first entry of `Child_` below it.
	template <typename Info_, u32 Parent_, u32 Child_>
	[[nodiscard]] consteval Array<u32, Info_::entry_count> child_table() {
		Array<u32, Info_::entry_count> table {};
		for (u32 parent = 0u; parent < Info_::entry_count; ++parent) {
			table[parent] = ~0u;
			if (Info_::slot_of(parent) != Parent_) {
				continue;
			}
			for (u32 entry = 0u; entry < Info_::entry_count; ++entry) {
				if (Info_::slot_of(entry) == Child_ && descends<Info_>(entry, parent)) {
					table[parent] = entry;
					break;
				}
			}
		}
		return table;
	}

	template <typename Info_, u32 Slot_, typename Byte_>
	class ArrayView;

	template <typename Info_, u32 Slot_, typename Byte_>
	[[nodiscard]] decltype(auto) at_entry(Byte_* const actor_, const u32 entry_) noexcept {
		if constexpr (slot_count<Info_>(Slot_) == 1u) {
			using type = like_t<Byte_, slot_type_t<Info_, Slot_>>;
			return *std::launder(reinterpret_cast<type*>(actor_ - Info_::actor_relative(entry_)));
		} else {
			return ArrayView<Info_, Slot_, Byte_> { actor_, entry_ };
		}
	}

	/**
	 * One element of an array slot; `get<…>` resolves relative to that element's sub-block.
	 */
	template <typename Info_, u32 Slot_, typename Byte_>
	class SlotView {
	public:
		using value_type = like_t<Byte_, slot_type_t<Info_, Slot_>>;

		constexpr SlotView(Byte_* const actor_, const u32 entry_) noexcept :
			_actor(actor_),
			_entry(entry_) {}

	private:
		Byte_* _actor;
		u32 _entry;

	public:
		[[nodiscard]] ref<value_type> operator*() const noexcept {
			return *std::launder(reinterpret_cast<value_type*>(_actor - Info_::actor_relative(_entry)));
		}

		[[nodiscard]] ptr<value_type> operator->() const noexcept {
			return std::addressof(**this);
		}

		template <FixedString Rel_>
		[[nodiscard]] decltype(auto) get() const noexcept {
			constexpr auto match = child_slot<Info_, Rel_>(Slot_);
			static_assert(match.matches != 0u, "No slot below the array element matches the path.");
			static_assert(match.matches < 2u, "Ambiguous slot name below the array element; qualify it with its path.");
			static_assert(
				slot_total<Info_>(match.slot) == slot_total<Info_>(Slot_) * slot_count<Info_>(match.slot),
				"Path crosses a nested array; index that array first."
			);

			constexpr static auto table = child_table<Info_, Slot_, match.slot>();
			return at_entry<Info_, match.slot>(_actor, table[_entry]);
		}
	};

	/**
	 * Fixed-size view of an array slot below one parent; elements are not contiguous (children sit in between).
	 */
	template <typename Info_, u32 Slot_, typename Byte_>
	class ArrayView {
	public:
		constexpr ArrayView(Byte_* const actor_, const u32 first_) noexcept :
			_actor(actor_),
			_first(first_) {}

	private:
		Byte_* _actor;
		u32 _first;

	public:
		[[nodiscard]] constexpr static size_t size() noexcept {
			return slot_count<Info_>(Slot_);
		}

		[[nodiscard]] SlotView<Info_, Slot_, Byte_> operator[](const size_t index_) const noexcept {
			::hg::assertd(index_ < size());
			return { _actor, _first + static_cast<u32>(index_) * element_stride<Info_>(Slot_) };
		}
	};

	template <typename Archetype_, FixedString Path_, typename Byte_>
	[[nodiscard]] decltype(auto) get(Byte_* const actor_) noexcept {
		using info = engine::acs::LayoutInfo<typename Archetype_::layout, Archetype_>;
		constexpr u32 entry = info::template index_of<Path_>();
		constexpr u32 slot = info::slot_of(entry);
		static_assert(
			slot_total<info>(slot) == slot_count<info>(slot),
			"Slot sits inside an array; reach it through the element: get<\"array\">()[i].get<\"name\">()."
		);
		return at_entry<info, slot>(actor_, entry);
	}
}

namespace hg {
	/**
	 * Inheritance helper of archetype actors: `InheritMeta` plus the archetype machinery.
	 *
	 * `Derived_` declares `using layout = ActorLayout<…>` (or `Extend<Base::layout, …>`). One `.cpp` per archetype
	 * includes `ArchetypeDefinition.inl.hpp` and explicitly instantiates `template class InheritArchetype<…>;`.
	 */
	template <typename Derived_, typename Base_, typename... Rest_>
	class InheritArchetype :
		public InheritMeta<Derived_, Base_, Rest_...> {
	public:
		using InheritMeta<Derived_, Base_, Rest_...>::InheritMeta;

	public:
		/**
		 * Defined in `ArchetypeDefinition.inl.hpp`; a missing explicit instantiation is a link error.
		 */
		[[nodiscard]] static ref<const TypedActorClass<Derived_>> actorClass() noexcept;

		/**
		 * Component of slot `Path_` resolved against `Derived_::layout`: actor address minus a constant.
		 *
		 * @returns `Type&` for single slots, a fixed-size view of `SlotView`s for arrays.
		 */
		template <FixedString Path_>
		[[nodiscard]] decltype(auto) get() noexcept {
			return archetype_detail::get<Derived_, Path_>(reinterpret_cast<std::byte*>(static_cast<Derived_*>(this)));
		}

		template <FixedString Path_>
		[[nodiscard]] decltype(auto) get() const noexcept {
			return archetype_detail::get<Derived_, Path_>(
				reinterpret_cast<const std::byte*>(static_cast<const Derived_*>(this))
			);
		}
	};
}
