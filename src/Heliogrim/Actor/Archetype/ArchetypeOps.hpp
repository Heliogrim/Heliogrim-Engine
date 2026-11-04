#pragma once

#include <cstddef>
#include <Engine.Common/Types.hpp>
#include <Engine.Common/Wrapper.hpp>

namespace hg::engine::acs {
	class Registry;
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
		void (*construct)(std::byte* slots_, u32 count_, u32 stride_, cref<ArchetypeInit> init_);
		// Actor first, then components in reverse entry order.
		void (*destroy)(std::byte* slots_, u32 count_, u32 stride_);
		// Moves live slots to uninitialised memory and re-stamps owner and parent links; `src_` is left dead.
		void (*relocate)(std::byte* src_, std::byte* dst_, u32 count_, u32 stride_);
		// nullptr unless the archetype itself declares `tick`.
		void (*tick)(std::byte* slots_, u32 count_, u32 stride_);

		// shortcut: serialization entries stay nullptr with placeholder signatures; Task 10 fills and types them.
		void (*serialize)(const std::byte* slot_, void* record_);
		void (*hydrate)(std::byte* slot_, const void* record_);
		void (*dumpLayout)(const std::byte* slots_, u32 count_, u32 stride_, void* stream_);
		void (*loadLayout)(std::byte* slots_, u32 count_, u32 stride_, const void* stream_);
	};
}
