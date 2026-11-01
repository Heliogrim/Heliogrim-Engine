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
	 * - Profile modules without ir are legacy text and are skipped.
	 * - Vertex offsets for lowering come from the stage intermediate: inbound variables carrying both a
	 *   SymbolId and a VkBindLocation annotation.
	 * - shortcut: the stage side offers every bind input and requires nothing, so only a missing bind class and
	 *   unknown target symbols are contract errors; add the real provides/requires once a stage declares them.
	 */
	[[nodiscard]] std::expected<smr<const lang::ir::Resolved>, lang::ir::Diagnostics> resolveStageIr(
		cref<lang::Intermediate> stage_,
		cref<EffectProfile> profile_,
		cref<EffectSpecification> spec_,
		cref<StageFlagBits> flags_,
		ref<lang::ir::VariantCache> cache_
	);

	/**
	 * `resolveStageIr` plus a fresh derivat owning a copy of the lowered intermediate.
	 */
	[[nodiscard]] std::expected<smr<StageDerivat>, lang::ir::Diagnostics> composeStageIr(
		cref<lang::Intermediate> stage_,
		cref<EffectProfile> profile_,
		cref<EffectSpecification> spec_,
		StageFlagBits flags_,
		ref<lang::ir::VariantCache> cache_
	);

	/**
	 * Appends the resolved layout of one stage: groups are descriptor sets (element order = binding order, a slot
	 * an earlier stage already took is skipped), push ranges carry the stage flag. Symbols come from `derivat_`.
	 */
	void appendBindLayout(
		cref<lang::ir::Resolved> resolved_,
		cref<StageDerivat> derivat_,
		ref<BindLayout> layout_
	);
}
