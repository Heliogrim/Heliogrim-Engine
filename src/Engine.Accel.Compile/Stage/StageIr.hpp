#pragma once

#include <expected>
#include <Engine.Accel.Effect/Stage/StageFlags.hpp>
#include <Engine.Accel.Lang/Intermediate.hpp>
#include <Engine.Accel.Lang/Ir/Diagnostic.hpp>
#include <Engine.Accel.Lang/Ir/Variant.hpp>
#include <Engine.Accel.Pipeline/Bind/BindLayout.hpp>
#include <Engine.Accel.Pipeline/Stage/StageDerivat.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>

namespace hg::engine::accel {
	class EffectProfile;
	class EffectSpecification;
}

namespace hg::engine::accel {
	/**
	 * Resolves one IR stage: merges the profile modules' ir and the stage ir (in that order), fixes the profile
	 * toggles, resolves, slices to the specification's target symbols (empty: all), checks the contract, lays out
	 * and lowers. The result is cached by content, so a hit is a key computation and one lookup; no module is merged,
	 * resolved or emitted again.
	 *
	 * - The specification's target symbols apply to the export stage only (`exportStage_`, the last stage of the
	 *   pipeline); earlier stages keep all their outputs.
	 * - Profile modules without ir are legacy text and are skipped (with a warning when they hold text).
	 * - Vertex offsets for lowering come from the stage intermediate: inbound variables carrying both a
	 *   SymbolId and a VkBindLocation annotation.
	 * - shortcut: the stage side offers every bind input and requires nothing, so contract checks are inert until
	 *   the pass supplies provides/requires; only a missing bind class and unknown target symbols fail today.
	 */
	[[nodiscard]] std::expected<smr<const lang::ir::Resolved>, lang::ir::Diagnostics> resolveStageIr(
		ref<const lang::Intermediate> stage_,
		ref<const EffectProfile> profile_,
		ref<const EffectSpecification> spec_,
		ref<const StageFlagBits> flags_,
		ref<lang::ir::VariantCache> cache_,
		bool exportStage_ = true
	);

	/**
	 * `resolveStageIr` plus a fresh derivat owning a copy of the lowered intermediate.
	 */
	[[nodiscard]] std::expected<smr<StageDerivat>, lang::ir::Diagnostics> composeStageIr(
		ref<const lang::Intermediate> stage_,
		ref<const EffectProfile> profile_,
		ref<const EffectSpecification> spec_,
		StageFlagBits flags_,
		ref<lang::ir::VariantCache> cache_,
		bool exportStage_ = true
	);

	/**
	 * Appends the resolved layout of one stage: groups are descriptor sets, push ranges carry the stage flag.
	 * Symbols come from `derivat_`. The pipeline compiler numbers bindings by position in the group, so an error
	 * is returned when
	 * - an element's position in its group is not the slot's binding (two ids on one slot, or a gap), or
	 * - an id an earlier stage appended sits at another (set, binding) here.
	 * shortcut: layouts are assigned per stage and only checked here; the fix is one layout pass over the union
	 * of all stages' bind entries deduplicated by id (`existing_` reuses by class and type key, so feeding it the
	 * sibling layouts would deliberately alias different ids).
	 */
	[[nodiscard]] lang::ir::Diagnostics appendBindLayout(
		ref<const lang::ir::Resolved> resolved_,
		ref<const StageDerivat> derivat_,
		ref<BindLayout> layout_
	);
}
