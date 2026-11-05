#pragma once

#include <cstddef>
#include <span>
#include <Engine.Common/Types.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/Vector.hpp>

namespace hg::engine::acs {
	class Registry;
}

namespace hg::engine::serialization {
	class StructScopedSlot;
}

namespace hg {
	/**
	 * Input to `ArchetypeOps::construct`; every actor of the span gets a new actor guid and component guids.
	 */
	struct ArchetypeInit {
		ref<engine::acs::Registry> registry;
	};

	/**
	 * Type-erased operations over a span of archetype slots `(slots, count, stride)`.
	 *
	 * One indirect call per span, never per actor. Slots start at the slot base, the actor sits at
	 * `ClusterDesc::actorOffset` within each slot.
	 */
	struct ArchetypeOps {
		// Components in entry order (parents before children), then the actor.
		void (*construct)(ptr<std::byte> slots_, u32 count_, u32 stride_, cref<ArchetypeInit> init_);

		// Actor first, then components in reverse entry order.
		void (*destroy)(ptr<std::byte> slots_, u32 count_, u32 stride_);

		// Moves live slots to uninitialised memory and re-stamps owner and parent links; `src_` is left dead.
		void (*relocate)(ptr<std::byte> src_, ptr<std::byte> dst_, u32 count_, u32 stride_);

		// nullptr unless the archetype itself declares `tick`.
		void (*tick)(ptr<std::byte> slots_, u32 count_, u32 stride_);

		// Writes the component records of one slot under `Symbols::Components` of the actor record, keyed by entry
		// path ("body/wheels/2/hub").
		void (*serialize)(ptr<const std::byte> slot_, ref<engine::serialization::StructScopedSlot> record_);

		// Reads the component records written by `serialize` into a constructed slot.
		void (*hydrate)(ptr<std::byte> slot_, ref<const engine::serialization::StructScopedSlot> record_);

		// Appends header {layoutHash, count, stride}, then per slot each component by memcpy or `access::Layout<T>`.
		void (*dumpLayout)(ptr<const std::byte> slots_, u32 count_, u32 stride_, ref<Vector<std::byte>> out_);

		// Loads a dump into constructed slots; false on header (hash, count, stride) mismatch or truncated input.
		bool (*loadLayout)(ptr<std::byte> slots_, u32 count_, u32 stride_, std::span<const std::byte> in_);
	};
}
