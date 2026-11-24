#include "FunctionTier.hpp"

#include <Engine.Driver.Vulkan/VkNativeBatch.hpp>
#include <Engine.Driver.Vulkan/VkRCmdTranslator.hpp>

#include "FunctionTarget.hpp"

import Engine.Accel.Compile;
import Engine.Accel.Effect;
import Engine.Accel.Lang;
import Engine.Accel.Pass;
import Engine.Accel.Pipeline;
import Engine.Common;
import Engine.Diagnostics;
import Engine.Graphics;
import Engine.Graphics.Render.Command;
import Engine.Graphics.RenderGraph;

using namespace hg::test::gpu;
using namespace hg::engine::accel;
using namespace hg::engine;
using namespace hg;

namespace {
	// One triangle covering NDC [-1, 1]^2; the overhang is clipped, so each texel shades exactly once.
	inline constexpr static StringView coverVertexAsl = R"(
const POSITIONS: f32vec2[3] = f32vec2[3](
	f32vec2(-1.0, -1.0),
	f32vec2(3.0, -1.0),
	f32vec2(-1.0, 3.0)
);

fn main() {
	position = f32vec4(POSITIONS[vertex_index], 0.0, 1.0);
}
)";

	// The single cover triangle drawn by `coverVertexAsl`.
	inline constexpr static u32 coverVertexCount = 3uL;

	struct Parsed {
		smr<lang::Intermediate> intermediate;
		String diagnostics;
	};

	[[nodiscard]] Opt<String> readShaderInclude(const StringView include_);

	[[nodiscard]] Parsed parseStage(const StringView name_, const StringView source_, const lang::ir::EntryKind entry_);

	[[nodiscard]] smr<const render::graph::Symbol> makeColorSymbol();

	[[nodiscard]] EffectCompileResult compileEffect(smr<const AccelerationEffect> effect_, ref<const smr<const GraphicsPass>> pass_);

	void draw(
		ref<const sptr<graphics::Device>> device_,
		ref<const smr<const GraphicsPass>> pass_,
		ref<const smr<const AccelerationPipeline>> pipeline_,
		ref<FunctionTarget> target_
	);

	[[nodiscard]] Raster expectedRaster(ref<const FunctionTestDesc> desc_);

	[[nodiscard]] CheckResult gpuRan(ref<const Raster> actual_);

	void finish(ref<const Report> report_, ref<const Raster> actual_);
}

Report hg::test::gpu::runFunction(ref<const sptr<graphics::Device>> device_, ref<const FunctionTestDesc> desc_) {
	FunctionTarget target { device_, desc_.extent };
	target.clear(math::fvec4 { sentinel });

	const auto expected = expectedRaster(desc_);
	const auto previous = store::loadPrevious(desc_.name);
	const auto* const previousPtr = previous.has_value() ? &*previous : nullptr;

	auto vertex = parseStage("gpu-test-cover.vs", coverVertexAsl, lang::ir::EntryKind::eVertex);
	auto fragment = parseStage(desc_.name, desc_.fragmentAsl, lang::ir::EntryKind::eFragment);

	String notes = vertex.diagnostics + fragment.diagnostics;
	EffectCompileResult compiled {};
	smr<const GraphicsPass> pass {};

	if (vertex.intermediate != nullptr && fragment.intermediate != nullptr) {
		auto vertexStage = make_smr<Stage>(StageFlagBits::eVertex);
		auto fragmentStage = make_smr<Stage>(StageFlagBits::eFragment);
		vertexStage->setIntermediate(std::move(vertex.intermediate));
		fragmentStage->setIntermediate(std::move(fragment.intermediate));

		Guid guid;
		GuidGenerate(guid);
		auto effect = make_smr<AccelerationEffect>(
			std::move(guid),
			"gpu-test-effect",
			Vector<smr<Stage>> { std::move(vertexStage), std::move(fragmentStage) }
		);

		// The target is an input as well, so the pass loads the sentinel clear instead of clearing it again.
		const auto color = makeColorSymbol();
		pass = VkAccelerationPassFactory {}.buildGraphicsPass({ color }, { color }).value();
		compiled = compileEffect(std::move(effect), clone(pass));
		if (compiled.pipeline == nullptr) {
			notes += "effect compile failed; the composer logged its diagnostics\n";
		}
	}

	if (compiled.pipeline == nullptr) {
		const auto actual = target.read();
		auto report = makeReport(
			desc_.name,
			actual,
			expected,
			{ CheckResult { "compile", false, 1u, 0.F, {} } },
			desc_.inputs,
			previousPtr
		);
		report.notes = std::move(notes);
		finish(report, actual);
		return report;
	}

	/**/

	draw(device_, pass, compiled.pipeline, target);
	compiled.pipeline.reset();
	pass.reset();

	const auto actual = target.read();
	auto checks = check(desc_.contract, actual, expected);
	checks.insert(checks.begin(), gpuRan(actual));

	auto report = makeReport(desc_.name, actual, expected, std::move(checks), desc_.inputs, previousPtr);
	finish(report, actual);
	return report;
}

/**/

namespace {
	[[nodiscard]] Opt<String> readShaderInclude(const StringView include_) {
		const auto base = std::filesystem::path { HG_TEST_SHADER_ROOT }.lexically_normal();
		const auto name = std::filesystem::path { String { include_ } };
		// Note: Includes are module-controlled input; never resolve outside the shader base.
		const auto candidate = (base / name).lexically_normal();
		const auto relative = candidate.lexically_relative(base);
		if (name.is_absolute() || relative.empty() || *relative.begin() == "..") {
			return None;
		}

		std::ifstream in { candidate, std::ios::binary };
		if (not in) {
			return None;
		}

		std::stringstream text;
		text << in.rdbuf();
		return SomeVal<String>(text.str());
	}

	[[nodiscard]] Parsed parseStage(const StringView name_, const StringView source_, const lang::ir::EntryKind entry_) {
		using namespace accel::lang;

		auto parsed = ir::parseModule(name_, source_, entry_, ir::ModuleResolver { readShaderInclude });
		if (not parsed.has_value()) {
			String text;
			for (const auto& diagnostic : parsed.error()) {
				text += diag::render(diagnostic, nullptr);
			}
			return { nullptr, std::move(text) };
		}

		auto intermediate = make_smr<Intermediate>();
		intermediate->ir = make_smr<const ir::Module>(new ir::Module { std::move(parsed).value() });

		// Mirror the declared interface so effect-level symbol enumeration sees it, as `makeIrIntermediate` does.
		auto& scope = intermediate->rep.globalScope;
		for (auto& entry : ir::computeInterface(*intermediate->ir)) {
			if (entry.implicit) {
				continue;
			}

			auto var = make_uptr<Variable>();
			var->annotation = make_uptr<SymbolIdAnnotation>(std::move(entry.id));
			(entry.input ? scope.inbound : scope.outbound).emplace_back(std::move(var));
		}
		return { std::move(intermediate), {} };
	}

	[[nodiscard]] smr<const render::graph::Symbol> makeColorSymbol() {
		using namespace render;
		auto description = make_smr<graph::TextureDescription>(
			graph::DescriptionValue { graph::DescriptionValueMatchingMode::eCovariant, graphics::TextureType::e2d },
			graph::DescriptionValue { graph::DescriptionValueMatchingMode::eInvariant, graphics::TextureFormat::eR32G32B32A32Sfloat },
			graph::DescriptionValue { graph::DescriptionValueMatchingMode::eInvariant, 1u },
			graph::DescriptionValue { graph::DescriptionValueMatchingMode::eIgnored, graph::ActiveMipBitMask {} }
		);
		return make_smr<graph::Symbol>(
			graph::SymbolFlagBits::eUndefined,
			graph::SymbolScope { .type = graph::SymbolScopeType::eGlobal, .layer = "" },
			"gpu-test::color",
			clone(description).into<const graph::Description>()
		);
	}

	[[nodiscard]] EffectCompileResult compileEffect(smr<const AccelerationEffect> effect_, ref<const smr<const GraphicsPass>> pass_) {
		auto profile = make_smr<EffectProfile>(EffectProfile { ._name = "gpu-test-profile", ._definitions = {} });

		// No target symbols: the compiler matches them against the effect's exports by object identity, not by id.
		auto spec = make_smr<SimpleEffectSpecification>(Vector<smr<const lang::Symbol>> {});
		spec->setPassSpec(
			make_uptr<GraphicsPassSpecification>(
				GraphicsPassSpecification {
					.depthCompareOp = DepthCompareOp::eNever,
					.stencilCompareOp = StencilCompareOp::eNever,
					.stencilFailOp = StencilOp::eKeep,
					.stencilPassOp = StencilOp::eKeep,
					.stencilDepthFailOp = StencilOp::eKeep,
					.stencilCompareMask = 0ul,
					.stencilWriteMask = 0ul,
					.primitiveTopology = PrimitiveTopology::eTriangleList,
					.pass = pass_.get()
				}
			)
		);

		return makeVkAccCompiler()->compile({ .effect = std::move(effect_), .profile = std::move(profile), .spec = std::move(spec) });
	}

	void draw(
		ref<const sptr<graphics::Device>> device_,
		ref<const smr<const GraphicsPass>> pass_,
		ref<const smr<const AccelerationPipeline>> pipeline_,
		ref<FunctionTarget> target_
	) {
		auto framebuffer = make_smr<graphics::Framebuffer>(device_);
		framebuffer->addAttachment(target_.texture());
		framebuffer->setExtent(target_.texture()->extent());
		framebuffer->setRenderPass(clone(pass_));
		framebuffer->setup();

		// Drain the queue on every path, so the framebuffer, pass and pipeline are never released while in flight.
		struct Drain {
			ref<const sptr<graphics::Device>> device;
			ref<const smr<graphics::Framebuffer>> framebuffer;

			// Non-throwing wait: a lost device must fail this test through the checked wait below, not terminate the run.
			~Drain() {
				static_cast<void>(vkQueueWaitIdle(device->graphicsQueue()->vkQueue()));
				framebuffer->destroy();
			}
		} drain { device_, framebuffer };

		/**/

		render::cmd::RenderCommandBuffer cmd {};
		cmd.begin();
		cmd.beginAccelPass({ pass_.get(), framebuffer.get() });
		cmd.beginSubPass();
		cmd.bindGraphicsPipeline(clone(pipeline_).into<GraphicsPipeline>());
		cmd.drawDispatch(1uL, 0uL, coverVertexCount, 0uL);
		cmd.endSubPass();
		cmd.endAccelPass();
		cmd.end();

		auto translator = make_uptr<driver::vk::VkRCmdTranslator>();
		auto nativeBatch = (*translator)(&cmd);
		static_cast<ptr<driver::vk::VkNativeBatch>>(nativeBatch.get())->commitAndDispose();
		nativeBatch.reset();

		device_->graphicsQueue()->vkQueue().waitIdle();
	}

	[[nodiscard]] Raster expectedRaster(ref<const FunctionTestDesc> desc_) {
		Raster raster { desc_.extent, Vector<math::fvec4>(static_cast<size_t>(desc_.extent.x) * desc_.extent.y) };
		for (u32 y = 0; y < desc_.extent.y; ++y) {
			for (u32 x = 0; x < desc_.extent.x; ++x) {
				raster.texels[static_cast<size_t>(y) * desc_.extent.x + x] = desc_.expected(x, y);
			}
		}
		return raster;
	}

	[[nodiscard]] CheckResult gpuRan(ref<const Raster> actual_) {
		const auto written = std::ranges::count_if(
			actual_.texels,
			[](ref<const math::fvec4> t_) { return t_ != math::fvec4 { sentinel }; }
		);
		return CheckResult { "gpu-ran", written > 0, written > 0 ? 0u : 1u, static_cast<f32>(written), {} };
	}

	void finish(ref<const Report> report_, ref<const Raster> actual_) {
		store::writeReport(report_);

		if (report_.passed()) {
			store::savePassing(report_.test, actual_);
		}
	}
}
