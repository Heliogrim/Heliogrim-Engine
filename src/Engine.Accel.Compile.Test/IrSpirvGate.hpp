#pragma once

#include <algorithm>
#include <array>
#include <expected>
#include <functional>
#include <span>
#include <initializer_list>
#include <string>
#include <utility>

#include <Engine.Accel.Compile/Profile/EffectProfile.hpp>
#include <Engine.Accel.Compile/Spec/EffectSpecification.hpp>
#include <Engine.Accel.Compile/Stage/StageIr.hpp>
#include <Engine.Accel.Lang/Ir/Parser.hpp>
#include <Engine.Accel.Lang/Ir/Interface.hpp>
#include <Engine.Accel.Lang/Ir/Spirv/SpirvValidate.hpp>
#include <Engine.Accel.Lang/Rep/Annotation.hpp>
#include <Engine.Common/Make.hpp>
#include <Engine.Diagnostics/Render.hpp>
#include <Engine.Logging/Logger.hpp>

#include "gtest/gtest.h"

namespace AccelCompile {
	class TestSpecification final :
		public hg::engine::accel::EffectSpecification {
	public:
		explicit TestSpecification(const hg::Vector<hg::String>& targets_) {
			for (const auto& target : targets_) {
				auto symbol = hg::make_smr<hg::engine::accel::lang::Symbol>(new hg::engine::accel::lang::Symbol {});
				symbol->symbolName = target;
				_targets.emplace_back(std::move(symbol).into<const hg::engine::accel::lang::Symbol>());
			}
		}

		[[nodiscard]] hg::ref<const hg::Vector<hg::smr<const hg::engine::accel::lang::Symbol>>> targetSymbols() const noexcept override {
			return _targets;
		}

		[[nodiscard]] hg::engine::accel::ComputePassSpecification getComputeSpec() const override {
			return {};
		}

		[[nodiscard]] hg::engine::accel::GraphicsPassSpecification getGraphicsSpec() const override {
			return {};
		}

		[[nodiscard]] hg::engine::accel::MeshPassSpecification getMeshSpec() const override {
			return {};
		}

		[[nodiscard]] hg::engine::accel::RaytracingPassSpecification getRaytracingSpec() const override {
			return {};
		}

	private:
		hg::Vector<hg::smr<const hg::engine::accel::lang::Symbol>> _targets;
	};

	/* in-memory include table: { name, source } */
	using IncludeTable = std::initializer_list<std::pair<hg::StringView, hg::StringView>>;

	/* diagnostics report through the engine logger */
	inline void ensureLogger() {
		static const bool ready = (hg::Logger::setup(), true);
		(void)ready;
	}

	using GateResolver = std::function<hg::Opt<hg::String>(hg::StringView)>;

	struct GateStage {
		hg::StringView source;
		hg::engine::accel::lang::ir::EntryKind entry;
	};

	/* parses and composes the stages as one pipeline; one stage is its own pipeline */
	[[nodiscard]] inline std::expected<hg::Vector<hg::smr<hg::engine::accel::StageDerivat>>, std::string> composeStages(
		std::span<const GateStage> stages_,
		hg::ref<const hg::engine::accel::lang::ir::ToggleSet> toggles_,
		hg::ref<const GateResolver> resolver_
	) {
		using namespace hg::engine::accel::lang::ir;
		using namespace hg::engine::accel;

		ensureLogger();

		hg::Vector<lang::Intermediate> stages {};
		stages.reserve(stages_.size());
		hg::Vector<StageFlagBits> flags {};
		for (const auto& gate : stages_) {
			auto parsed = parseModule("M", gate.source, gate.entry, ModuleResolver { resolver_ });
			if (not parsed.has_value()) {
				std::string failure = "parse failed:";
				for (const auto& d : parsed.error()) {
					failure += "\n" + std::string { hg::engine::diag::render(d) };
				}
				return std::unexpected(std::move(failure));
			}

			auto& stage = stages.emplace_back();
			stage.ir = hg::make_smr<const Module>(new Module { std::move(parsed).value() });

			/* the pass supplies vertex offsets; any distinct ones do for the gate */
			if (gate.entry == EntryKind::eVertex) {
				hg::u32 offset = 0uL;
				for (const auto& entry : computeInterface(*stage.ir)) {
					if (entry.input and entry.mode == BindMode::eForward) {
						auto var = hg::make_uptr<lang::Variable>();
						var->annotation = hg::make_uptr<lang::VkBindLocationAnnotation>(
							-1L, 0L, static_cast<hg::s32>(offset), hg::make_uptr<lang::SymbolIdAnnotation>(hg::String { entry.id }), -1L
						);
						stage.rep.globalScope.inbound.emplace_back(std::move(var));
						offset += 16uL;
					}
				}
			}

			switch (gate.entry) {
				case EntryKind::eFragment: flags.emplace_back(StageFlagBits::eFragment); break;
				case EntryKind::eCompute: flags.emplace_back(StageFlagBits::eCompute); break;
				case EntryKind::eGeometry: flags.emplace_back(StageFlagBits::eGeometry); break;
				case EntryKind::eTessCtrl: flags.emplace_back(StageFlagBits::eTessellationCtrl); break;
				case EntryKind::eTessEval: flags.emplace_back(StageFlagBits::eTessellationEval); break;
				case EntryKind::eVertex:
				default: flags.emplace_back(StageFlagBits::eVertex); break;
			}
		}

		EffectProfile profile {};
		profile._name = "Test";
		profile._toggles = toggles_;
		const TestSpecification spec { {} };
		VariantCache cache {};
		PipelineLayoutCache layouts {};

		hg::Vector<const lang::Intermediate*> pointers {};
		for (const auto& stage : stages) {
			pointers.emplace_back(&stage);
		}
		auto composed = composePipelineIr(pointers, flags, profile, spec, cache, layouts);
		if (not composed.has_value()) {
			std::string failure = "resolve failed:";
			for (const auto& d : composed.error()) {
				failure += "\n" + std::string { hg::engine::diag::render(d) };
			}
			return std::unexpected(std::move(failure));
		}
		return std::move(composed).value();
	}

	/* the lowered stage carries spir-v that passes the validator */
	[[nodiscard]] inline ::testing::AssertionResult spirvValid(hg::ref<const hg::engine::accel::lang::Intermediate> lowered_) {
		if (lowered_.spirv.empty()) {
			return ::testing::AssertionFailure() << "no spir-v emitted";
		}
		const auto diags = hg::engine::accel::lang::ir::spirv::validate(lowered_.spirv, "M");
		if (diags.empty()) {
			return ::testing::AssertionSuccess();
		}
		auto failure = ::testing::AssertionFailure() << "spirv-val rejected the module:";
		for (const auto& d : diags) {
			failure << "\n" << std::string { hg::engine::diag::render(d) };
		}
		return failure;
	}

	[[nodiscard]] inline ::testing::AssertionResult compilesToSpirv(
		hg::StringView source_,
		hg::engine::accel::lang::ir::EntryKind entry_,
		hg::ref<const hg::engine::accel::lang::ir::ToggleSet> toggles_ = {},
		IncludeTable includes_ = {}
	) {
		const GateResolver resolver = [includes_](hg::StringView name_) -> hg::Opt<hg::String> {
			const auto it = std::ranges::find(includes_, name_, &std::pair<hg::StringView, hg::StringView>::first);
			if (it == includes_.end()) {
				return hg::None;
			}
			return hg::String { it->second };
		};
		const std::array stages { GateStage { source_, entry_ } };
		const auto composed = composeStages(stages, toggles_, resolver);
		if (not composed.has_value()) {
			return ::testing::AssertionFailure() << composed.error();
		}
		return spirvValid(*composed->front()->getIntermediate());
	}
}
