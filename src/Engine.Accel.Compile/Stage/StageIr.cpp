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

			auto layout = assignLayout(interfaceEntries, sliced.types, {});
			layout.specConstants = assignSpecConstants(sliced.constants);

			DenseMap<String, u32> vertexOffsets {};
			DenseMap<String, s32> vertexFormats {};
			for (const auto& [id, offset, format] : offsets) {
				vertexOffsets.insert_or_assign(id, offset);
				if (format >= 0L) {
					vertexFormats.insert_or_assign(id, format);
				}
			}

			auto lowered = lowerToIntermediate(sliced, layout, Dialect::eVulkanGlsl460, vertexOffsets, vertexFormats);
			if (not lowered.has_value()) {
				failure = std::move(lowered).error();
				return {};
			}

			return make_smr<const Resolved>(
				new Resolved {
					std::move(sliced),
					std::move(layout),
					std::move(interfaceEntries),
					std::move(lowered).value()
				}
			);
		}
	);

	if (resolved == nullptr) {
		return std::unexpected(std::move(failure));
	}
	return resolved;
}

std::expected<smr<StageDerivat>, Diagnostics> hg::engine::accel::composeStageIr(
	ref<const Intermediate> stage_,
	ref<const EffectProfile> profile_,
	ref<const EffectSpecification> spec_,
	const StageFlagBits flags_,
	ref<VariantCache> cache_,
	const bool exportStage_
) {

	auto resolved = resolveStageIr(stage_, profile_, spec_, flags_, cache_, exportStage_);
	if (not resolved.has_value()) {
		return std::unexpected(std::move(resolved).error());
	}

	auto derivat = make_smr<StageDerivat>(
		nmpt<Stage> {},
		Vector<smr<Stage>> {},
		smr<StageModule> { nullptr },
		StageFlagBits { flags_ }
	);
	derivat->setIntermediate(make_smr<Intermediate>((*resolved)->lowered));
	return derivat;
}

Diagnostics hg::engine::accel::appendBindLayout(
	ref<const Resolved> resolved_,
	ref<const StageDerivat> derivat_,
	ref<BindLayout> layout_
) {

	const auto intermediate = derivat_.getIntermediate();

	for (const auto& slot : resolved_.layout.slots) {

		const auto iter = std::ranges::find_if(
			intermediate->rep.symbolTable,
			[&](ref<const uptr<Symbol>> s_) { return s_->symbolName == slot.id; }
		);
		if (iter == intermediate->rep.symbolTable.end()) {
			continue;
		}

		/* an id an earlier stage appended must sit at the same (set, binding) here */
		bool known = false;
		for (size_t set = 0uLL; set < layout_.groups.size(); ++set) {
			const auto& elements = layout_.groups[set].elements;
			for (size_t binding = 0uLL; binding < elements.size(); ++binding) {
				if (elements[binding].symbol->symbolName != slot.id) {
					continue;
				}
				if (set != slot.set || binding != slot.binding) {
					return fail(
						"binding '" + slot.id + "' is at set " + std::to_string(set) + ", binding " +
						std::to_string(binding) + " in an earlier stage but at set " + std::to_string(slot.set) +
						", binding " + std::to_string(slot.binding) + " here"
					);
				}
				known = true;
			}
		}
		if (known) {
			continue;
		}

		if (layout_.groups.size() <= slot.set) {
			layout_.groups.resize(slot.set + 1uLL);
		}

		/* the pipeline compiler numbers bindings by position in the group */
		auto& elements = layout_.groups[slot.set].elements;
		if (elements.size() != slot.binding) {
			return fail(
				"binding '" + slot.id + "' at set " + std::to_string(slot.set) + ", binding " +
				std::to_string(slot.binding) + " does not line up with the bindings of the other stages"
			);
		}
		elements.emplace_back(BindElement { nmpt<const Symbol> { iter->get() }, bindTypeOf(resolved_, slot.id) });
	}

	for (const auto& range : resolved_.layout.push) {

		auto existing = std::ranges::find_if(
			layout_.constants,
			[&](ref<const PushConstant> c_) { return c_.offset == range.offset && c_.size == range.size; }
		);

		if (existing != layout_.constants.end()) {
			existing->stages |= derivat_.getFlagBits();
		} else {
			layout_.constants.emplace_back(derivat_.getFlagBits(), range.offset, range.size);
		}
	}

	return {};
}
