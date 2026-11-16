#pragma once

#include <expected>
#include <Engine.Common/String.hpp>
#include <Engine.Common/Types.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/DenseMap.hpp>

#include "Diagnostic.hpp"
#include "Layout.hpp"
#include "Module.hpp"
#include "Tier.hpp"
#include "../Intermediate.hpp"

namespace hg::engine::accel::lang::ir {
	/**
	 * Builds a fresh legacy intermediate from a resolved (and sliced) module and its layout; nothing is shared with
	 * an earlier lowering except the immutable `ir` copy of the module.
	 * - `spirv`: the module `spirv::emit` builds at `tier_`; `interface`: `describeInterface` of `resolved_`.
	 * - `rep`: one variable per interface entry (inputs inbound, outputs outbound) and its symbol (`SymbolId` of the
	 *   id). Annotation chains, outermost first: forward `SymbolId -> ForwardLinkage`, preceded by
	 *   `VkBindLocation(-1, location, offset)` for a vertex input (offset from `vertexOffsets_` by id, `vk::Format` from `vertexFormats_` by id, absent: -1); bind
	 *   `SymbolId -> Storage | Uniform -> ExternalLinkage` (textures and samplers are uniform); push
	 *   `SymbolId -> Readonly -> Constant -> ExternalLinkage`. Implicit entries get a variable and symbol too.
	 * Errors: every emitter error, a vertex input without offset. Emitter warnings land in `warnings`.
	 */
	[[nodiscard]] std::expected<Intermediate, Diagnostics> lowerToIntermediate(
		ref<const Module> resolved_,
		ref<const Layout> layout_,
		Tier tier_,
		ref<const DenseMap<String, u32>> vertexOffsets_ = {},
		ref<const DenseMap<String, s32>> vertexFormats_ = {}
	);
}
