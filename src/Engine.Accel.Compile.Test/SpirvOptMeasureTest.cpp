#include "pch.h"

/**/

#include <chrono>
#include <spirv-tools/optimizer.hpp>

#include "IrSpirvGate.hpp"

using namespace hg::engine::accel::lang::ir;

namespace AccelCompile {
	namespace {
		struct Variant {
			std::size_t words;
			double ms;
			bool valid;
		};

		enum class Passes { eNone, ePerformance, eSize };

		/* min of five runs; the output of the last run is kept */
		[[nodiscard]] Variant measure(hg::ref<const hg::Vector<hg::u32>> in_, Passes passes_) {
			if (passes_ == Passes::eNone) {
				return { in_.size(), 0.0, spirv::validate(in_, "M").empty() };
			}
			double best = std::numeric_limits<double>::max();
			hg::Vector<hg::u32> out {};
			bool ran = true;
			for (int i = 0; i < 5; ++i) {
				spvtools::Optimizer opt { SPV_ENV_VULKAN_1_3 };
				passes_ == Passes::ePerformance ? opt.RegisterPerformancePasses() : opt.RegisterSizePasses();
				out.clear();
				const auto begin = std::chrono::steady_clock::now();
				ran = opt.Run(in_.data(), in_.size(), &out);
				const auto end = std::chrono::steady_clock::now();
				best = std::min(best, std::chrono::duration<double, std::milli>(end - begin).count());
			}
			if (not ran) {
				return { 0uz, best, false };
			}
			return { out.size(), best, spirv::validate(std::span<const hg::u32> { out.data(), out.size() }, "M").empty() };
		}
	}

	TEST(SpirvOptBuild, EngineShadersOptimizedAreValid) {
		const GateResolver resolver { readShader };
		const auto shaders = engineShaders();
		ASSERT_FALSE(shaders.empty());

		for (const auto& [name, entry] : shaders) {
			const auto source = readShader(name);
			const std::array stages { GateStage { *source, entry } };
			const auto composed = composeStages(stages, {}, resolver, Tier::eBuild);
			ASSERT_TRUE(composed.has_value()) << name << ": " << composed.error();
			for (const auto& diag : composed->front()->getIntermediate()->warnings) {
				EXPECT_NE(diag.code()->key, "internal-spirv-optimize") << name;
			}
			const auto& spirv = (*composed->front()->getIntermediate()).spirv;
			ASSERT_FALSE(spirv.empty()) << name;
			const auto diags = spirv::validate(spirv, name);
			EXPECT_TRUE(diags.empty()) << name << ": " << diags.front().message();
		}
	}

	/* the build tier output is already optimized, so 'none' measures the shipped module */
	TEST(DISABLED_SpirvOptMeasure, EngineShaders) {
		const GateResolver resolver { readShader };
		const auto shaders = engineShaders();
		ASSERT_FALSE(shaders.empty());

		std::printf(
			"| shader | words none | words perf | words size | opt ms perf | opt ms size | valid none | valid perf | valid size |\n"
			"|---|---:|---:|---:|---:|---:|:---:|:---:|:---:|\n"
		);
		std::size_t none = 0, perf = 0, size = 0;
		double perfMs = 0.0, sizeMs = 0.0;
		for (const auto& [name, entry] : shaders) {
			const auto source = readShader(name);
			const std::array stages { GateStage { *source, entry } };
			const auto composed = composeStages(stages, {}, resolver, Tier::eBuild);
			ASSERT_TRUE(composed.has_value()) << name << ": " << composed.error();
			const auto& spirv = (*composed->front()->getIntermediate()).spirv;

			const auto a = measure(spirv, Passes::eNone);
			const auto b = measure(spirv, Passes::ePerformance);
			const auto c = measure(spirv, Passes::eSize);
			std::printf(
				"| %s | %zu | %zu | %zu | %.2f | %.2f | %s | %s | %s |\n",
				name.c_str(),
				a.words,
				b.words,
				c.words,
				b.ms,
				c.ms,
				a.valid ? "yes" : "NO",
				b.valid ? "yes" : "NO",
				c.valid ? "yes" : "NO"
			);
			EXPECT_TRUE(a.valid) << name << ": unoptimized output fails validation";
			EXPECT_TRUE(b.valid) << name << ": spirv-opt (performance) output fails validation";
			EXPECT_TRUE(c.valid) << name << ": spirv-opt (size) output fails validation";
			none += a.words;
			perf += b.words;
			size += c.words;
			perfMs += b.ms;
			sizeMs += c.ms;
		}
		std::printf("| **total** | %zu | %zu | %zu | %.2f | %.2f | | | |\n", none, perf, size, perfMs, sizeMs);
	}
}
