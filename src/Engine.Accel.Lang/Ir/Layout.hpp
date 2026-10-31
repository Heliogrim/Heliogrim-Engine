#pragma once

#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/String.hpp>
#include <Engine.Common/Types.hpp>
#include <Engine.Common/Wrapper.hpp>

#include "Interface.hpp"
#include "Module.hpp"

namespace hg::engine::accel::lang::ir {
	/**
	 * Costs the layout pass trades against each other; only their ratios matter.
	 */
	struct LayoutWeights {
		/* per binding number left unused to line a binding up with an existing layout */
		float unusedSlotCost = 1.0f;
		/* per binding that gets a new number instead of an identical existing one */
		float extraLayoutCost = 4.0f;
		/* for opening one more descriptor set */
		float extraSetCost = 8.0f;
	};

	struct SlotAssignment {
		String id;
		u32 set = 0uL;
		u32 binding = 0uL;
		/* identity for reuse across shaders: class and structural type key (empty: type not laid out, never reused) */
		String identity;

		[[nodiscard]] bool operator==(const SlotAssignment&) const noexcept = default;
	};

	struct PushRange {
		String id;
		u32 offset = 0uL;
		u32 size = 0uL;

		[[nodiscard]] bool operator==(const PushRange&) const noexcept = default;
	};

	/**
	 * `slots` sorted by (set, binding); `push` in id order; `inputLocations[i]` is the first location of the i-th
	 * forward input in id order. Forward outputs get no entry: the caller numbers them by id.
	 */
	struct Layout {
		Vector<SlotAssignment> slots;
		Vector<PushRange> push;
		Vector<u32> inputLocations;

		[[nodiscard]] bool operator==(const Layout&) const noexcept = default;
	};

	/**
	 * std430 member offsets of a struct type; empty if the type is not a struct or not laid out (see below).
	 */
	[[nodiscard]] Vector<u32> std430Offsets(ref<const Vector<TypeDesc>> types_, TypeRef struct_);

	/**
	 * Locations a forward variable of the type takes, counted as for forward inputs below (at least 1).
	 */
	[[nodiscard]] u32 locationCount(ref<const Vector<TypeDesc>> types_, TypeRef type_);

	/**
	 * Resolve order step 5. Deterministic: the same entries (in any order) and type table contents (in any order)
	 * with the same weights and existing layouts give the same layout; everything iterates sorted vectors.
	 *
	 * Entries: duplicates collapse by id, the first in (id, class, type key, size, locations, valid) order wins (bind
	 * and push share one id space, forward inputs their own). A bind entry with class `eNone` and every forward output
	 * are skipped (no slot or location; the caller's contract check must report a missing class). Bind entries take descriptor slots, push entries push ranges, forward inputs locations.
	 *
	 * Slots: ordered by class (uniform, storage, texture, sampler), then id, so a fresh layout is packed: set 0,
	 * bindings 0..n-1 with each class contiguous.
	 * shortcut: the IR carries no update frequency, so all bindings start in set 0 grouped by class; add a
	 * frequency annotation (per pipeline / per pass / per invocation) and key sets by it when materials need it.
	 * 1. Reuse: for an entry, the slots of `existing_` with the same class and type key that this layout has not
	 *    taken are candidates. Repeatedly, the (entry, candidate) pair with fewest skipped bindings (binding minus
	 *    the set's next free binding, or 0 when below it) is taken, ties by same id, then (set, binding), then entry
	 *    order; it is reused only if `unusedSlotCost * skipped < extraLayoutCost`, else reuse stops.
	 * 2. New bindings (entries not reused), same order: once, before the first one, a new set (one past the highest
	 *    set in use) is opened if `extraSetCost` is below the accumulated skip cost, `unusedSlotCost` times the
	 *    numbers below set 0's next free binding that no reuse took; new bindings then append at the
	 *    next free binding of that set (or of set 0). Skipped numbers stay free: they keep the reused set lined up
	 *    with the existing layout.
	 *
	 * Push ranges: packed in id order, each offset aligned to max(type alignment, 4), each size the std430 struct
	 * size rounded up to 4 (Vulkan granularity). A type that cannot be laid out gets size 0.
	 *
	 * std430 (type table, depth limit 256 like the parser; cyclic, out-of-range, too deep or over-4GiB types cannot
	 * be laid out; results are memoized per type):
	 * - scalar: bits/8 bytes (8, 16, 32 or 64 bits), bool 4; alignment = size.
	 * - vector n (2-4): size n * c, alignment (n == 3 ? 4 : n) * c for component size c (vec3: size 12, align 16).
	 * - matrix n columns x m rows (2-4): column-major array of n vec m (mat4: 64 bytes, align 16; mat3: 48).
	 * - array: stride = element size rounded up to element alignment, size count * stride, alignment of the
	 *   element; count 0 (unsized) only as the last member of a block and never in a push range.
	 * - struct: members at offsets rounded up to their alignment, alignment the max member alignment, size the end
	 *   rounded up to it (no vec4 rounding as in std140).
	 * - texture, sampler: opaque, cannot be a member.
	 *
	 * Locations of forward inputs: ascending from 0 in id order; scalar and vector take 1 (a 64-bit vec3/vec4: 2),
	 * matrix one per column, array count * element (unsized: one element), anything else 1.
	 */
	[[nodiscard]] Layout assignLayout(
		ref<const Vector<InterfaceEntry>> entries_,
		ref<const Vector<TypeDesc>> types_,
		ref<const LayoutWeights> weights_,
		ref<const Vector<Layout>> existing_ = {}
	);
}
