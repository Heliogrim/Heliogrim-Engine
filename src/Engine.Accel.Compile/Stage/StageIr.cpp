#include "StageIr.hpp"

#include <algorithm>
#include <Engine.Accel.Lang/Dialect.hpp>
#include <Engine.Accel.Lang/Ir/Contract.hpp>
#include <Engine.Accel.Lang/Ir/Interface.hpp>
#include <Engine.Accel.Lang/Ir/Layout.hpp>
#include <Engine.Accel.Lang/Ir/Lower.hpp>
#include <Engine.Accel.Lang/Ir/Merge.hpp>
#include <Engine.Accel.Lang/Ir/Resolve.hpp>
#include <Engine.Accel.Lang/Ir/Slice.hpp>
#include <Engine.Accel.Lang/Ir/Typecheck.hpp>
#include <Engine.Accel.Lang/Rep/Annotation.hpp>
#include <Engine.Common/Make.hpp>
#include <Engine.Logging/Logger.hpp>

#include "../Profile/EffectProfile.hpp"
#include "../Spec/EffectSpecification.hpp"

using namespace hg;
using namespace hg::engine::accel;
using namespace hg::engine::accel::lang;
using namespace hg::engine::accel::lang::ir;

ModuleTargetStage hg::engine::accel::toModuleTargetStage(const EntryKind entry_) noexcept {
	switch (entry_) {
		case EntryKind::eVertex: return ModuleTargetStage::eVertex;
		case EntryKind::eTessCtrl: return ModuleTargetStage::eTessCtrl;
		case EntryKind::eTessEval: return ModuleTargetStage::eTessEval;
		case EntryKind::eGeometry: return ModuleTargetStage::eGeometry;
		case EntryKind::eFragment: return ModuleTargetStage::eFragment;
		case EntryKind::eCompute: return ModuleTargetStage::eCompute;
	}
	return ModuleTargetStage::eUndefined;
}

namespace {
	[[nodiscard]] Diagnostics fail(String message_) {
		return Diagnostics { Diagnostic { .severity = Severity::eError, .message = std::move(message_) } };
	}

	[[nodiscard]] bool hasError(ref<const Diagnostics> diagnostics_) {
		return std::ranges::any_of(
			diagnostics_,
			[](ref<const Diagnostic> d_) { return d_.severity == Severity::eError; }
		);
	}

	/* inbound variables carrying a SymbolId and a VkBindLocation: (id, vertex offset, vertex format or -1), sorted */
	[[nodiscard]] Vector<std::tuple<String, u32, s32>> vertexOffsetsOf(ref<const Intermediate> stage_) {
		Vector<std::tuple<String, u32, s32>> offsets {};
		for (const auto& var : stage_.rep.globalScope.inbound) {

			const SymbolIdAnnotation* id = nullptr;
			const VkBindLocationAnnotation* location = nullptr;
			for (auto* cur = var->annotation.get(); cur != nullptr; cur = cur->next.get()) {
				if (cur->type == AnnotationType::eSymbolId) {
					id = static_cast<const SymbolIdAnnotation*>(cur);
				} else if (cur->type == AnnotationType::eVkBindLocation) {
					location = static_cast<const VkBindLocationAnnotation*>(cur);
				}
			}

			if (id != nullptr && location != nullptr && location->vkOffset >= 0) {
				offsets.emplace_back(String { id->symbolId }, static_cast<u32>(location->vkOffset), location->vkFormat);
			}
		}

		std::ranges::sort(offsets);
		return offsets;
	}

	[[nodiscard]] BindType bindTypeOf(ref<const Resolved> resolved_, ref<const String> id_) {
		for (const auto& entry : resolved_.interfaceEntries) {
			if (entry.input && entry.id == id_) {
				switch (entry.cls) {
					case BindClass::eStorageBuffer: return BindType::eStorageBuffer;
					case BindClass::eTexture: return BindType::eTexture;
					case BindClass::eSampler: return BindType::eTextureSampler;
					case BindClass::eStorageImage: return BindType::eStorageImage;
					default: return BindType::eUniformBuffer;
				}
			}
		}
		return BindType::eUniformBuffer;
	}
}

std::expected<smr<const Resolved>, Diagnostics> hg::engine::accel::resolveStageIr(
	ref<const Intermediate> stage_,
	ref<const EffectProfile> profile_,
	ref<const EffectSpecification> spec_,
	ref<const StageFlagBits> flags_,
	ref<VariantCache> cache_,
	const bool exportStage_
) {

	if (stage_.ir == nullptr) {
		return std::unexpected(fail("stage has no ir"));
	}

	Vector<const Module*> modules {};
	modules.reserve(profile_._modules.size() + 1uLL);
	bool skippedText = false;
	for (const auto& module : profile_._modules) {
		if (module.ir != nullptr) {
			modules.emplace_back(module.ir.get());
		} else if (not module.lang.text.empty()) {
			skippedText = true;
		}
	}
	modules.emplace_back(stage_.ir.get());

	Vector<String> outputs {};
	if (exportStage_) {
		for (const auto& symbol : spec_.targetSymbols()) {
			outputs.emplace_back(symbol->symbolName);
		}
	}

	const auto offsets = vertexOffsetsOf(stage_);

	/* Key: the input modules, not the merged one, so a hit merges nothing. */
	auto key = makeVariantKey(*stage_.ir, profile_._toggles, outputs);
	{
		ContentHasher h {};
		h.put(static_cast<u64>(modules.size()));
		for (const auto* module : modules) {
			h.put(contentHash(*module));
		}
		h.put(flags_);
		h.put(static_cast<u64>(offsets.size()));
		for (const auto& [id, offset, format] : offsets) {
			h.put(StringView { id });
			h.put(offset);
			h.put(format);
		}
		key.module = h.value;
	}

	Diagnostics failure {};
	auto resolved = cache_.getOrCreate(
		key,
		[&]() -> smr<const Resolved> {

			/* non-fatal parser/typecheck diagnostics; merge drops them, so log them from the inputs */
			for (const auto* module : modules) {
				for (const auto& warning : module->warnings) {
					Logger::warn("Ir stage module '{}': {}", module->name, warning.message);
				}
			}

			if (skippedText) {
				Logger::warn("Ir stage skips profile '{}' modules without ir; their text is not emitted", profile_._name);
			}

			auto merged = merge(modules);
			if (not merged.has_value()) {
				failure = std::move(merged).error();
				return {};
			}

			failure = checkStageValidity(*merged, stage_.ir->entry);
			if (hasError(failure)) {
				return {};
			}
			failure.clear();

			auto fixed = fixToggles(*merged, ToggleSources { .pass = {}, .profile = profile_._toggles, .material = {} });
			if (not fixed.has_value()) {
				failure = std::move(fixed).error();
				return {};
			}

			auto flat = resolveToggles(*merged, fixed->toggles);
			if (not flat.has_value()) {
				failure = std::move(flat).error();
				return {};
			}

			/* unknown target symbols are errors, the slice would silently keep nothing */
			const auto all = computeInterface(*flat);
			for (const auto& id : outputs) {
				if (std::ranges::none_of(all, [&](ref<const InterfaceEntry> e_) { return not e_.input && e_.id == id; })) {
					failure = fail("target symbol '" + id + "' is not an output of the stage");
					return {};
				}
			}

			auto sliced = slice(*flat, outputs);
			auto interfaceEntries = computeInterface(sliced);

			StageContract contract {};
			for (const auto& entry : interfaceEntries) {
				if (entry.input && entry.mode == BindMode::eBind) {
					contract.provides.emplace_back(entry.id);
				}
			}

			failure = checkContract(interfaceEntries, contract, {});
			if (hasError(failure)) {
				return {};
			}
			for (const auto& warning : failure) {
				Logger::warn("Ir stage contract: {}", warning.message);
			}
			failure.clear();

			return make_smr<const Resolved>(new Resolved { std::move(sliced), std::move(interfaceEntries), key });
		}
	);

	if (resolved == nullptr) {
		return std::unexpected(std::move(failure));
	}
	return resolved;
}

std::expected<Vector<smr<StageDerivat>>, Diagnostics> hg::engine::accel::composePipelineIr(
	std::span<const Intermediate* const> stages_,
	std::span<const StageFlagBits> flags_,
	ref<const EffectProfile> profile_,
	ref<const EffectSpecification> spec_,
	ref<VariantCache> cache_,
	ref<PipelineLayoutCache> layouts_,
	smr<const ResolvedPipeline>* pipeline_,
	const bool exportLast_
) {

	if (stages_.empty()) {
		return std::unexpected(fail("pipeline has no ir stages"));
	}
	if (stages_.size() != flags_.size()) {
		return std::unexpected(fail("stage and flag counts differ"));
	}

	Vector<smr<const Resolved>> resolved {};
	Vector<VariantKey> key {};
	resolved.reserve(stages_.size());
	key.reserve(stages_.size());
	for (size_t idx = 0uLL; idx < stages_.size(); ++idx) {
		if (stages_[idx] == nullptr) {
			return std::unexpected(fail("stage has no ir"));
		}
		/* target symbols are the effect's exports: only the effect's last stage is sliced to them */
		auto stage = resolveStageIr(
			*stages_[idx], profile_, spec_, flags_[idx], cache_, exportLast_ && idx + 1uLL == stages_.size()
		);
		if (not stage.has_value()) {
			return std::unexpected(std::move(stage).error());
		}
		key.emplace_back((*stage)->key);
		resolved.emplace_back(std::move(stage).value());
	}

	Diagnostics failure {};
	auto pipeline = layouts_.getOrCreate(
		key,
		[&]() -> smr<const ResolvedPipeline> {
			Vector<Vector<InterfaceEntry>> entries {};
			Vector<decltype(Module::types)> types {};
			Vector<decltype(Module::constants)> constants {};
			for (const auto& stage : resolved) {
				entries.emplace_back(stage->interfaceEntries);
				types.emplace_back(stage->module.types);
				constants.emplace_back(stage->module.constants);
			}

			auto layouts = assignPipelineLayout(entries, types, {});
			if (not layouts.has_value()) {
				failure = std::move(layouts).error();
				return {};
			}
			auto specs = assignPipelineSpecConstants(constants, types);
			if (not specs.has_value()) {
				failure = std::move(specs).error();
				return {};
			}

			Vector<Intermediate> lowered {};
			lowered.reserve(resolved.size());
			for (size_t idx = 0uLL; idx < resolved.size(); ++idx) {
				(*layouts)[idx].specConstants = std::move((*specs)[idx]);

				DenseMap<String, u32> vertexOffsets {};
				DenseMap<String, s32> vertexFormats {};
				for (const auto& [id, offset, format] : vertexOffsetsOf(*stages_[idx])) {
					vertexOffsets.insert_or_assign(id, offset);
					if (format >= 0L) {
						vertexFormats.insert_or_assign(id, format);
					}
				}

				auto stage = lowerToIntermediate(
					resolved[idx]->module,
					(*layouts)[idx],
					Dialect::eVulkanGlsl460,
					vertexOffsets,
					vertexFormats
				);
				if (not stage.has_value()) {
					failure = std::move(stage).error();
					return {};
				}
				lowered.emplace_back(std::move(stage).value());
			}

			return make_smr<const ResolvedPipeline>(
				new ResolvedPipeline { resolved, std::move(layouts).value(), std::move(lowered) }
			);
		}
	);

	if (pipeline == nullptr) {
		return std::unexpected(std::move(failure));
	}

	Vector<smr<StageDerivat>> derivats {};
	derivats.reserve(stages_.size());
	for (size_t idx = 0uLL; idx < stages_.size(); ++idx) {
		auto derivat = make_smr<StageDerivat>(
			nmpt<Stage> {},
			Vector<smr<Stage>> {},
			smr<StageModule> { nullptr },
			StageFlagBits { flags_[idx] }
		);
		derivat->setIntermediate(make_smr<Intermediate>(pipeline->lowered[idx]));
		derivats.emplace_back(std::move(derivat));
	}

	if (pipeline_ != nullptr) {
		*pipeline_ = std::move(pipeline);
	}
	return derivats;
}

Diagnostics hg::engine::accel::appendBindLayout(
	ref<const ResolvedPipeline> pipeline_,
	std::span<const smr<StageDerivat>> derivats_,
	ref<BindLayout> layout_
) {

	if (derivats_.size() != pipeline_.layouts.size()) {
		return fail("pipeline and derivat counts differ");
	}

	/* every distinct slot of the pipeline, in (set, binding) order */
	Vector<const SlotAssignment*> slots {};
	for (const auto& layout : pipeline_.layouts) {
		for (const auto& slot : layout.slots) {
			if (std::ranges::none_of(slots, [&](const SlotAssignment* s_) { return s_->id == slot.id; })) {
				slots.emplace_back(&slot);
			}
		}
	}
	std::ranges::sort(
		slots,
		[](const SlotAssignment* l_, const SlotAssignment* r_) {
			return std::tie(l_->set, l_->binding) < std::tie(r_->set, r_->binding);
		}
	);

	for (const auto* slot : slots) {

		/* the symbol comes from whichever stage declares the id */
		const Symbol* symbol = nullptr;
		size_t owner = 0uLL;
		for (size_t idx = 0uLL; idx < derivats_.size() && symbol == nullptr; ++idx) {
			if (not std::ranges::contains(pipeline_.layouts[idx].slots, slot->id, &SlotAssignment::id)) {
				continue;
			}
			const auto intermediate = derivats_[idx]->getIntermediate();
			const auto iter = std::ranges::find_if(
				intermediate->rep.symbolTable,
				[&](ref<const uptr<Symbol>> s_) { return s_->symbolName == slot->id; }
			);
			if (iter != intermediate->rep.symbolTable.end()) {
				symbol = iter->get();
				owner = idx;
			}
		}
		if (symbol == nullptr) {
			continue;
		}

		if (layout_.groups.size() <= slot->set) {
			layout_.groups.resize(slot->set + 1uLL);
		}

		/* the pipeline compiler numbers bindings by position in the group */
		auto& elements = layout_.groups[slot->set].elements;
		if (elements.size() != slot->binding) {
			return fail(
				"binding '" + slot->id + "' at set " + std::to_string(slot->set) + ", binding " +
				std::to_string(slot->binding) + " does not line up with the other bindings of the pipeline"
			);
		}
		elements.emplace_back(BindElement { nmpt<const Symbol> { symbol }, bindTypeOf(*pipeline_.stages[owner], slot->id) });
	}

	for (size_t idx = 0uLL; idx < derivats_.size(); ++idx) {
		for (const auto& range : pipeline_.layouts[idx].push) {

			auto existing = std::ranges::find_if(
				layout_.constants,
				[&](ref<const PushConstant> c_) { return c_.offset == range.offset && c_.size == range.size; }
			);

			if (existing != layout_.constants.end()) {
				existing->stages |= derivats_[idx]->getFlagBits();
			} else {
				layout_.constants.emplace_back(derivats_[idx]->getFlagBits(), range.offset, range.size);
			}
		}
	}

	return {};
}
