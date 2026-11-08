#pragma once

#include <algorithm>
#include <array>
#include <initializer_list>
#include <string>
#include <utility>

#include <Engine.Accel.Compile/Module/ModuleSource.hpp>
#include <Engine.Accel.Compile/Module/SpirvCompiler.hpp>
#include <Engine.Accel.Compile/Profile/EffectProfile.hpp>
#include <Engine.Accel.Compile/Spec/EffectSpecification.hpp>
#include <Engine.Accel.Compile/Stage/StageIr.hpp>
#include <Engine.Accel.Lang/Ir/Parser.hpp>
#include <Engine.Accel.Lang/Ir/Interface.hpp>
#include <Engine.Accel.Lang/Rep/Annotation.hpp>
#include <Engine.Common/Make.hpp>
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

	/* glslang accepts the glsl as the given stage and yields non-empty spir-v */
	[[nodiscard]] inline ::testing::AssertionResult glslCompiles(hg::StringView glsl_, hg::engine::accel::lang::ir::EntryKind entry_) {
		using namespace hg::engine::accel;

		/* glslang reports through the engine logger */
		static const bool loggerReady = (hg::Logger::setup(), true);
		(void)loggerReady;

		ModuleSource source {};
		source.targetStage = toModuleTargetStage(entry_);
		source.code.text.emplace_back(lang::TextBlock { hg::string { glsl_ } });

		const auto byteCode = SpirvCompiler {}.compile(source, source.code.text);
		if (byteCode.empty()) {
			return ::testing::AssertionFailure() << "glslang rejected the glsl:\n" << std::string { glsl_ };
		}
		return ::testing::AssertionSuccess();
	}

	[[nodiscard]] inline ::testing::AssertionResult compilesToSpirv(
		hg::StringView source_,
		hg::engine::accel::lang::ir::EntryKind entry_,
		hg::ref<const hg::engine::accel::lang::ir::ToggleSet> toggles_ = {},
		IncludeTable includes_ = {}
	) {
		using namespace hg::engine::accel;
		using namespace hg::engine::accel::lang::ir;

		const auto resolver = [includes_](hg::StringView name_) -> hg::Opt<hg::String> {
			const auto it = std::ranges::find(includes_, name_, &std::pair<hg::StringView, hg::StringView>::first);
			if (it == includes_.end()) {
				return hg::None;
			}
			return hg::String { it->second };
		};
		auto parsed = parseModule("M", source_, entry_, ModuleResolver { resolver });
		if (not parsed.has_value()) {
			auto failure = ::testing::AssertionFailure() << "parse failed:";
			for (const auto& d : parsed.error()) {
				failure << "\n" << d.line << ":" << d.column << " " << std::string { d.message };
			}
			return failure;
		}

		lang::Intermediate stage {};
		stage.ir = hg::make_smr<const Module>(new Module { std::move(parsed).value() });

		/* the pass supplies vertex offsets; any distinct ones do for the gate */
		if (entry_ == EntryKind::eVertex) {
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

		EffectProfile profile {};
		profile._name = "Test";
		profile._toggles = toggles_;
		const TestSpecification spec { {} };
		VariantCache cache {};
		PipelineLayoutCache layouts {};

		/* one stage is its own pipeline */
		const std::array<const lang::Intermediate*, 1> stages { &stage };
		const auto stageBit = [](EntryKind kind_) {
			switch (kind_) {
				case EntryKind::eFragment: return StageFlagBits::eFragment;
				case EntryKind::eCompute: return StageFlagBits::eCompute;
				case EntryKind::eGeometry: return StageFlagBits::eGeometry;
				case EntryKind::eTessCtrl: return StageFlagBits::eTessellationCtrl;
				case EntryKind::eTessEval: return StageFlagBits::eTessellationEval;
				default: return StageFlagBits::eVertex;
			}
		};
		const std::array<StageFlagBits, 1> flags { stageBit(entry_) };
		const auto composed = composePipelineIr(stages, flags, profile, spec, cache, layouts);
		if (not composed.has_value()) {
			auto failure = ::testing::AssertionFailure() << "resolve failed:";
			for (const auto& d : composed.error()) {
				failure << "\n" << d.line << ":" << d.column << " " << std::string { d.message };
			}
			return failure;
		}

		const auto& lowered = *composed->front()->getIntermediate();
		auto result = glslCompiles(lowered.lang.text.empty() ? hg::StringView {} : hg::StringView { lowered.lang.text.front().data }, entry_);
		if (not result) {
			return result << "\n(emitted)";
		}
		return result;
	}
}
