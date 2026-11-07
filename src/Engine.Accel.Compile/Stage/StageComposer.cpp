#include "StageComposer.hpp"

#include <map>
#include <ranges>

#include <Engine.Accel.Effect/AccelerationEffect.hpp>
#include <Engine.Accel.Pipeline/Stage/StageModule.hpp>
#include <Engine.Asserts/Breakpoint.hpp>
#include <Engine.Common/Make.hpp>
#include <Engine.Common/Sal.hpp>
#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>
#include <Engine.GFX.Render.Subpass/Impl/__tmp_helper.hpp>
#include <Engine.Logging/Logger.hpp>
#include <Engine.Pedantic/Clone/Clone.hpp>

#include "StageIr.hpp"
#include "../Profile/EffectProfile.hpp"
#include "../Profile/ProfileDefinition.hpp"
#include "../Spec/EffectSpecification.hpp"

using namespace hg::engine::accel;
using namespace hg;

static void preprocessStageDerivat(
	cref<smr<const Stage>> stage_,
	cref<smr<const class EffectProfile>> profile_,
	_Inout_ smr<StageDerivat> derivat_
);

/**/

non_owning_rptr<const Tokenizer> StageComposer::getTokenizer() const noexcept {
	return _tokenizer;
}

void StageComposer::setTokenizer(mref<non_owning_rptr<const Tokenizer>> tokenizer_) {
	_tokenizer = std::move(tokenizer_);
}

Vector<smr<StageDerivat>> StageComposer::compose(
	cref<smr<const EffectProfile>> profile_,
	cref<smr<const EffectSpecification>> specifications_,
	cref<smr<AccelerationPipeline>> targetPass_
) const {

	const auto& stages = targetPass_->getEffect()->getStages();

	Vector<smr<StageDerivat>> derivats {};
	derivats.reserve(stages.size());

	/* derivat index per ir stage, and the ir stages themselves; text-only stages are not in here */
	Vector<size_t> irIndices {};
	Vector<const lang::Intermediate*> irStages {};
	Vector<StageFlagBits> irFlags {};
	/* per ir stage: directly follows the previous ir stage in the effect (u8: no vector<bool> span) */
	Vector<u8> irFollows {};

	for (const auto& stage : stages) {

		StageFlagBits flagBits { stage->getFlagBits() };

		/**/

		derivats.push_back(
			make_smr<StageDerivat>(
				stage.get(),
				Vector<smr<Stage>> { stage },
				smr<StageModule> { nullptr },
				std::move(flagBits)
			)
		);

		const auto source = stage->getIntermediate();
		if (source != nullptr && source->ir != nullptr) {

			/* a legacy stage in between breaks the stage-chain check */
			irFollows.push_back(static_cast<u8>(not irIndices.empty() && irIndices.back() + 1uLL == derivats.size() - 1uLL));
			irIndices.emplace_back(derivats.size() - 1uLL);
			irStages.emplace_back(source.get());
			irFlags.emplace_back(stage->getFlagBits());
			continue;
		}

		preprocessStageDerivat(stage, profile_, derivats.back());
	}

	/* one layout for all ir stages; the last of them is sliced to the effect's exports */
	smr<const lang::ir::ResolvedPipeline> pipeline {};
	if (not irStages.empty()) {
		auto composed = composePipelineIr(
			irStages,
			irFlags,
			*profile_,
			*specifications_,
			_variants,
			_pipelineLayouts,
			&pipeline,
			irIndices.back() + 1uLL == derivats.size(),
			irFollows
		);
		if (not composed.has_value()) {
			for (const auto& diagnostic : composed.error()) {
				Logger::error(
					"Failed to compose ir stage: {} ({}:{})",
					diagnostic.message,
					diagnostic.line,
					diagnostic.column
				);
			}
			return {};
		}

		for (size_t irIdx = 0uLL; irIdx < irIndices.size(); ++irIdx) {
			derivats[irIndices[irIdx]]->setIntermediate((*composed)[irIdx]->getIntermediate());
		}
	}

	/**/

	BindLayout layout {};
	Vector<nmpt<const lang::Symbol>> symbols {};

	if (not irIndices.empty()) {
		Vector<smr<StageDerivat>> irDerivats {};
		for (const auto idx : irIndices) {
			irDerivats.emplace_back(derivats[idx]);
		}
		const auto clash = appendBindLayout(*pipeline, irDerivats, layout);
		if (not clash.empty()) {
			for (const auto& diagnostic : clash) {
				Logger::error("Failed to lay out ir stages: {}", diagnostic.message);
			}
			return {};
		}
	}

	size_t irCursor = 0uLL;
	for (size_t stageIdx = 0uLL; stageIdx < derivats.size(); ++stageIdx) {

		const auto& derivat = derivats[stageIdx];

		if (irCursor < irIndices.size() && irIndices[irCursor] == stageIdx) {
			++irCursor;
			continue;
		}

		symbols.clear();
		derivat->getIntermediate()->enumerateInboundSymbols(symbols);
		derivat->getIntermediate()->enumerateOutboundSymbols(symbols);

		/**/

		for (const auto& symbol : symbols) {

			BindElement element {
				symbol
			};

			/**/

			const auto storeType = [](ref<BindElement> dst_) {

				const auto var = dst_.symbol->var.data;
				const auto type = dst_.symbol->var.data->type;

				if (type.category == lang::TypeCategory::eTexture) {

					var->annotation;
					dst_.type = BindType::eTexture;

					return true;

				} else if (type.category == lang::TypeCategory::eObject && type.objectType ==
					lang::ObjectType::eSampler) {

					var->annotation;
					dst_.type = BindType::eTextureSampler;

					return true;

				} else if (type.category == lang::TypeCategory::eObject) {

					auto annotation = var->annotation.get();
					while (annotation != nullptr) {
						if (
							annotation->type == lang::AnnotationType::eStorage ||
							annotation->type == lang::AnnotationType::eUniform
						) {
							break;
						}
						annotation = annotation->next.get();
					}

					if (annotation != nullptr && annotation->type == lang::AnnotationType::eStorage) {
						dst_.type = BindType::eStorageBuffer;
					} else {
						dst_.type = BindType::eUniformBuffer;
					}
					return true;
				}
				// TODO: Handle Constants
				return false;
			};

			/**/

			assert(symbol->var.type == lang::SymbolType::eVariableSymbol);
			assert(symbol->var.data->annotation);

			bool hasExternalLinkage = false;

			auto* annotation = symbol->var.data->annotation.get();
			while (annotation != nullptr) {
				if (annotation->type == lang::AnnotationType::eExternalLinkage) {
					hasExternalLinkage = true;
					break;
				}
				annotation = annotation->next.get();
			}

			/**/

			if (not hasExternalLinkage) {
				continue;
			}

			const auto isPushConstant = [](cref<BindElement> el_) -> bool {

				const auto var = el_.symbol->var.data;
				auto annotation = var->annotation.get();
				while (annotation != nullptr) {
					if (annotation->type == lang::AnnotationType::eConstant) {
						return true;
					}
					annotation = annotation->next.get();
				}

				return false;
			};

			if (isPushConstant(element)) {

				u32 offset = 0uL;
				if (not layout.constants.empty()) {
					offset = layout.constants.back().offset + layout.constants.back().size;
				}

				layout.constants.emplace_back(
					derivat->getFlagBits(),
					offset,
					/* sizeof(...) */
					static_cast<u32>(sizeof(float)) * 2uL * 2uL
				);
				continue;
			}

			/**/

			storeType(element);

			/**/

			bool grouped = false;
			for (auto& candidate : layout.groups) {
				if (/* candidate.compatible(element) */false) {
					candidate.elements.push_back(std::move(element));
					grouped = true;
					break;
				}
			}

			if (not grouped) {
				auto group = BindGroup {};
				group.elements.push_back(std::move(element));
				layout.groups.push_back(std::move(group));
			}
		}
	}

	/**/

	targetPass_->setBindingLayout(std::move(layout));
	return derivats;
}

#include <sstream>

static void preprocessStageDerivat(
	cref<smr<const Stage>> stage_,
	cref<smr<const EffectProfile>> profile_,
	smr<StageDerivat> derivat_
) {

	static string shaderMacros = engine::render::read_shader_file(R"(shared/__macros__.glsl)");

	std::stringstream ss {};
	ss << "#version 450 core\n";

	switch (stage_->getFlagBits()) {
		case StageFlagBits::eVertex: {
			ss << "#define HG_SCTX_TYPE_VERTEX\n";
			break;
		}
		case StageFlagBits::eTessellationCtrl: {
			ss << "#define HG_SCTX_TYPE_TESSELLATION_CTRL\n";
			break;
		}
		case StageFlagBits::eTessellationEval: {
			ss << "#define HG_SCTX_TYPE_TESSELLATION_EVAL\n";
			break;
		}
		case StageFlagBits::eGeometry: {
			ss << "#define HG_SCTX_TYPE_GEOMETRY\n";
			break;
		}
		case StageFlagBits::eFragment: {
			ss << "#define HG_SCTX_TYPE_FRAGMENT\n";
			break;
		}
		case StageFlagBits::eCompute: {
			ss << "#define HG_SCTX_TYPE_COMPUTE\n";
			break;
		}
		default: {
			::hg::breakpoint();
		}
	}

	auto preamble = ss.str();

	/**/

	auto intermediate = make_smr<lang::Intermediate>(*stage_->getIntermediate());

	auto& il = intermediate->lang.text;
	std::reverse(il.begin(), il.end());

	/* Store injections and external definitions in reverse order */
	il.emplace_back(shaderMacros);

	for (auto iter = profile_->_definitions.rbegin(); iter != profile_->_definitions.rend(); ++iter) {

		const auto& definition = *iter;
		if (definition.type == DefinitionType::eText) {
			il.emplace_back(definition.data);
		}
	}

	for (const auto& module : profile_->_modules | std::views::reverse) {

		/* text-only stages concatenate module text; ir stages merge ir in `resolveStageIr` */
		for (const auto block : module.lang.text | std::views::reverse) {
			il.emplace_back(block);
		}
	}

	il.emplace_back(std::move(preamble));
	/**/

	/*
	 * Layout Inversion:
	 *
	 * | Preamble               |
	 * | ---------------------- |
	 * | Profile Definitions    |
	 * | ---------------------- |
	 * | Shader Macros          |
	 * | Shader Body            |
	 *
	 */
	std::reverse(il.begin(), il.end());

	derivat_->setIntermediate(std::move(intermediate));
}
