#pragma once

#include <expected>
#include <span>
#include <Engine.Accel.Effect/Stage/StageFlags.hpp>
#include <Engine.Accel.Lang/Intermediate.hpp>
#include <Engine.Accel.Lang/Ir/Diagnostic.hpp>
#include <Engine.Accel.Lang/Ir/Module.hpp>
#include <Engine.Accel.Lang/Ir/Variant.hpp>
#include <Engine.Accel.Pipeline/Bind/BindLayout.hpp>
#include <Engine.Accel.Pipeline/Stage/StageDerivat.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>

#include "../Module/ModuleTargetStage.hpp"

namespace hg::engine::accel {
	class EffectProfile;
	class EffectSpecification;
}

namespace hg::engine::accel {
	/**
	 * The glslang stage an IR entry kind is compiled as.
	 */
	[[nodiscard]] ModuleTargetStage toModuleTargetStage(lang::ir::EntryKind entry_) noexcept;

	/**
	 * Resolves one IR stage: merges the profile modules' ir and the stage ir (in that order), fixes the profile
	 * toggles, resolves, slices to the specification's target symbols (empty: all) and checks the contract. Layout
	 * and lowering need the other stages: see `composePipelineIr`. The result is cached by content, so a hit is a
	 * key computation and one lookup; no module is merged or resolved again.
	 *
	 * - The specification's target symbols apply to the export stage only (`exportStage_`, the last stage of the
	 *   effect, which may be a legacy text stage); other stages keep all their outputs.
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
	 * Composes the IR stages of one pipeline, in stage order: resolves each (`resolveStageIr`; the last stage is the
	 * export stage only if `exportLast_`, i.e. it is also the effect's last stage), lays all of them out in one pass (ids shared by stages get one slot, spec constants are
	 * numbered once per pipeline) and lowers each with that layout. The layout and the lowered stages are cached by
	 * the stages' variant keys in `layouts_`, so a hit lays out and lowers nothing. Returns one derivat per stage,
	 * owning a copy of its lowered intermediate. `pipeline_`, when given, receives the resolved pipeline (for
	 * `appendBindLayout`).
	 */
	[[nodiscard]] std::expected<Vector<smr<StageDerivat>>, lang::ir::Diagnostics> composePipelineIr(
		std::span<const lang::Intermediate* const> stages_,
		std::span<const StageFlagBits> flags_,
		ref<const EffectProfile> profile_,
		ref<const EffectSpecification> spec_,
		ref<lang::ir::VariantCache> cache_,
		ref<lang::ir::PipelineLayoutCache> layouts_,
		smr<const lang::ir::ResolvedPipeline>* pipeline_ = nullptr,
		bool exportLast_ = true
	);

	/**
	 * Builds the bind layout of a composed pipeline from all its stages at once (`derivats_[i]` is stage i of
	 * `pipeline_`): groups are descriptor sets, the union of the stages' slots sorted by (set, binding), each symbol
	 * taken from the first stage declaring it. Push ranges carry the flags of the stages using them. The pipeline
	 * compiler numbers bindings by position in the group, so an error is returned when a set has a gap, i.e. an
	 * element's position is not the slot's binding.
	 */
	[[nodiscard]] lang::ir::Diagnostics appendBindLayout(
		ref<const lang::ir::ResolvedPipeline> pipeline_,
		std::span<const smr<StageDerivat>> derivats_,
		ref<BindLayout> layout_
	);
}
