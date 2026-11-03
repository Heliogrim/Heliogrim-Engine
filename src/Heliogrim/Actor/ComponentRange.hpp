#pragma once

#include <bit>
#include <compare>
#include <cstddef>
#include <iterator>
#include <ranges>
#include <span>
#include <Engine.Asserts/Asserts.hpp>
#include <Engine.Common/Types.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/__warning.hpp>
#include <Engine.Common/Collection/Set.hpp>

namespace hg {
	/**
	 * Forward Declaration
	 */
	class HierarchyComponent;

	/**
	 * Random-access view over the components of one actor.
	 *
	 * Archetype actors: `base = slotBase`, entries = the shared offset table of each `HierarchyComponent` subobject.
	 * Dynamic actors: `base = 0`, entries = the pointers stored in the actor's `CompactSet`.
	 * Either way an element is `base + *entry`; only `contains` tells the two kinds apart.
	 *
	 * The view borrows the offset table, bitmap or set; it must not outlive them.
	 */
	// Raw entry pointers keep the iterator at two words, as the design asks; the bounds come from the caller.
	BEGIN_REGION_UNCHECKED_LLVL
	class ComponentRange : public std::ranges::view_interface<ComponentRange> {
	public:
		using value_type = ptr<HierarchyComponent>;
		using dynamic_set_type = CompactSet<ptr<HierarchyComponent>>;

		class iterator {
		public:
			using iterator_concept = std::random_access_iterator_tag;
			using iterator_category = std::random_access_iterator_tag;
			using value_type = ptr<HierarchyComponent>;
			using difference_type = std::ptrdiff_t;
			using reference = value_type;

			constexpr iterator() noexcept = default;

			constexpr iterator(const uintptr_t base_, const ptr<const uintptr_t> entry_) noexcept :
				_base(base_),
				_entry(entry_) {}

		private:
			uintptr_t _base = 0uLL;
			ptr<const uintptr_t> _entry = nullptr;

		public:
			[[nodiscard]] value_type operator*() const noexcept {
				// Dynamic entries are pointer objects, not uintptr_t objects; memcpy keeps the load free of aliasing UB.
				uintptr_t entry;
				std::memcpy(&entry, _entry, sizeof(entry));
				return std::bit_cast<value_type>(_base + entry);
			}

			[[nodiscard]] value_type operator[](const difference_type n_) const noexcept {
				return *(*this + n_);
			}

			iterator& operator++() noexcept {
				++_entry;
				return *this;
			}

			iterator operator++(int) noexcept {
				auto tmp = *this;
				++_entry;
				return tmp;
			}

			iterator& operator--() noexcept {
				--_entry;
				return *this;
			}

			iterator operator--(int) noexcept {
				auto tmp = *this;
				--_entry;
				return tmp;
			}

			iterator& operator+=(const difference_type n_) noexcept {
				_entry += n_;
				return *this;
			}

			iterator& operator-=(const difference_type n_) noexcept {
				_entry -= n_;
				return *this;
			}

			[[nodiscard]] friend iterator operator+(iterator it_, const difference_type n_) noexcept {
				return it_ += n_;
			}

			[[nodiscard]] friend iterator operator+(const difference_type n_, iterator it_) noexcept {
				return it_ += n_;
			}

			[[nodiscard]] friend iterator operator-(iterator it_, const difference_type n_) noexcept {
				return it_ -= n_;
			}

			[[nodiscard]] friend difference_type operator-(const iterator& lhs_, const iterator& rhs_) noexcept {
				return lhs_._entry - rhs_._entry;
			}

			// Position-only: `_base` is not part of identity (default-constructed sentinels have base 0).
			[[nodiscard]] friend bool operator==(const iterator& lhs_, const iterator& rhs_) noexcept {
				return lhs_._entry == rhs_._entry;
			}

			[[nodiscard]] friend std::strong_ordering operator<=>(const iterator& lhs_, const iterator& rhs_) noexcept {
				return lhs_._entry <=> rhs_._entry;
			}
		};

	public:
		constexpr ComponentRange() noexcept = default;

		/**
		 * @param slotBase_ Start of the actor's slot; the actor sits at `slotBase_ + blockSize_`.
		 * @param offsets_ Offset of each component's `HierarchyComponent` subobject from `slotBase_`.
		 * @param bitmap_ One bit per `minAlign_` step of the component block, set at every valid start offset.
		 * @param blockSize_ Byte size of the component block in front of the actor.
		 * @param minAlign_ Minimum component alignment, a power of two.
		 */
		[[nodiscard]] static ComponentRange archetype(
			std::byte* const slotBase_,
			const std::span<const uintptr_t> offsets_,
			const ptr<const u64> bitmap_,
			const u32 blockSize_,
			const u32 minAlign_
		) noexcept {
			::hg::assertrt(std::has_single_bit(minAlign_));

			ComponentRange range {};
			range._base = reinterpret_cast<uintptr_t>(slotBase_);
			range._entries = offsets_.data();
			range._count = offsets_.size();
			range._bitmap = bitmap_;
			range._blockSize = blockSize_;
			range._alignShift = static_cast<u32>(std::countr_zero(minAlign_));
			return range;
		}

		[[nodiscard]] static ComponentRange dynamic(cref<dynamic_set_type> set_) noexcept {
			static_assert(sizeof(ptr<HierarchyComponent>) == sizeof(uintptr_t));

			const auto& values = set_.values();

			ComponentRange range {};
			range._entries = reinterpret_cast<const uintptr_t*>(values.data());
			range._count = values.size();
			range._set = &set_;
			return range;
		}

		static ComponentRange dynamic(mref<dynamic_set_type>) = delete;

	private:
		uintptr_t _base = 0uLL;
		ptr<const uintptr_t> _entries = nullptr;
		size_t _count = 0uLL;

		/* Dynamic only; nullptr marks an archetype range. */
		ptr<const dynamic_set_type> _set = nullptr;

		/* Archetype only. */
		ptr<const u64> _bitmap = nullptr;
		u32 _blockSize = 0u;
		u32 _alignShift = 0u;

	public:
		[[nodiscard]] iterator begin() const noexcept {
			return iterator { _base, _entries };
		}

		[[nodiscard]] iterator end() const noexcept {
			return iterator { _base, _entries + _count };
		}

		[[nodiscard]] size_t size() const noexcept {
			return _count;
		}

		/**
		 * O(1) membership test. Archetype: range check `[slotBase, actor)` plus one bitmap bit; an address inside
		 * a component that is not a start offset is not a member. Dynamic: hash lookup.
		 */
		[[nodiscard]] bool contains(const ptr<const HierarchyComponent> component_) const noexcept {
			if (_set != nullptr) {
				return _set->contains(const_cast<ptr<HierarchyComponent>>(component_));
			}

			// Unsigned wrap sends addresses below slotBase (and nullptr) past blockSize.
			const auto offset = reinterpret_cast<uintptr_t>(component_) - _base;
			if (offset >= _blockSize || (offset & ((1uLL << _alignShift) - 1uLL)) != 0uLL) {
				return false;
			}

			const auto bit = offset >> _alignShift;
			return ((_bitmap[bit >> 6uLL] >> (bit & 63uLL)) & 1uLL) != 0uLL;
		}
	};

	END_REGION_UNCHECKED_LLVL
}
