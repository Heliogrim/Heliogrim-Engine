#include "pch.h"

#include <Engine.Accel.Lang/Emitter/GlslEmitter.hpp>
#include <Engine.Accel.Lang/Ir/Interface.hpp>
#include <Engine.Accel.Lang/Ir/Layout.hpp>
#include <Engine.Accel.Lang/Ir/Parser.hpp>
#include <Engine.Accel.Lang/Ir/Variant.hpp>

using namespace hg;
using namespace hg::engine::accel::lang;
using namespace hg::engine::accel::lang::ir;

namespace AccelLang {
	namespace {
		[[nodiscard]] Opt<String> noIncludes(StringView) {
			return None;
		}

		[[nodiscard]] std::expected<Module, Diagnostics> parse(StringView source_, EntryKind entry_ = EntryKind::eCompute) {
			return parseModule("M", source_, entry_, ModuleResolver { noIncludes });
		}

		[[nodiscard]] Module parseOk(StringView source_, EntryKind entry_ = EntryKind::eCompute) {
			auto res = parse(source_, entry_);
			EXPECT_TRUE(res.has_value()) << (res.has_value() || res.error().empty() ? "" : res.error().front().message);
			return res.has_value() ? std::move(res).value() : Module {};
		}

		[[nodiscard]] String errorOf(StringView source_, EntryKind entry_ = EntryKind::eCompute) {
			auto res = parse(source_, entry_);
			return res.has_value() || res.error().empty() ? String {} : res.error().front().message;
		}

		[[nodiscard]] String emitGlsl(ref<Module> module_) {
			Language target {};
			const auto layout = assignLayout(computeInterface(module_), module_.types, {});
			(void)GlslEmitter {}.emit(module_, layout, target);
			return target.text.empty() ? String {} : String { target.text.front().data };
		}

		constexpr auto buffer = "@input (mode = bind, id = \"buf\", class = storage, access = read_write) block buf { u32[] data; };\n";
		constexpr auto image = "@input (mode = bind, id = \"img\", class = image, access = read) image2d<rgba16f> img;\n";
	}

	TEST(ComputeTest, ParsesLocalSize) {
		const auto module = parseOk("@compute (local_size = [8, 4, 2])\nfn main() {}\n");
		EXPECT_EQ(module.attributes.localSize, (std::array<u32, 3> { 8u, 4u, 2u }));
		EXPECT_EQ(parseOk("fn main() {}\n").attributes.localSize, (std::array<u32, 3> { 1u, 1u, 1u }));
	}

	TEST(ComputeTest, LocalSizeOnFragmentIsDiagnostic) {
		EXPECT_FALSE(errorOf("@compute (local_size = [8, 8, 1])\nfn main() {}\n", EntryKind::eFragment).empty());
	}

	TEST(ComputeTest, WorkgroupVariableEmitsShared) {
		auto module = parseOk("workgroup tile: f32[64];\n@compute (local_size = [64, 1, 1])\nfn main() {\n\ttile[local_invocation_index] = 1.0;\n}\n");
		const auto glsl = emitGlsl(module);
		EXPECT_TRUE(glsl.contains("shared ")) << glsl;
		EXPECT_TRUE(glsl.contains("local_size_x = 64")) << glsl;
	}

	TEST(ComputeTest, BarrierInFragmentIsDiagnostic) {
		EXPECT_FALSE(errorOf("fn main() {\n\tbarrier();\n}\n", EntryKind::eFragment).empty());
	}

	TEST(ComputeTest, ImageStoreNeedsWriteAccess) {
		const auto store = "fn main() {\n\timage_store(img, s32vec2(0, 0), f32vec4(1.0));\n}\n";
		EXPECT_FALSE(errorOf(String { image } + store).empty());
		const auto writable = String { "@input (mode = bind, id = \"img\", class = image, access = write) image2d<rgba16f> img;\n" } + store;
		EXPECT_EQ(errorOf(writable), "") << errorOf(writable);
	}

	TEST(ComputeTest, AtomicNeedsMutablePlace) {
		EXPECT_EQ(errorOf(String { buffer } + "fn main() {\n\tlet old = atomic_add(buf.data[0], 1u);\n}\n"), "");
		EXPECT_FALSE(errorOf("fn main() {\n\tlet old = atomic_add(1u, 1u);\n}\n").empty());
	}

	TEST(ComputeTest, BarrierUnderInvocationIdBranchIsError) {
		const auto message = errorOf("fn main() {\n\tif (local_invocation_id.x > 3u) {\n\t\tbarrier();\n\t}\n}\n");
		EXPECT_EQ(message, "'barrier' under lane-dependent control; lanes that skip it deadlock the workgroup");
	}

	TEST(ComputeTest, BarrierUnderWorkgroupIdBranchIsValid) {
		EXPECT_EQ(errorOf("fn main() {\n\tif (workgroup_id.x > 3u) {\n\t\tbarrier();\n\t}\n}\n"), "");
	}

	TEST(ComputeTest, BarrierAfterDivergentReturnIsError) {
		const auto message = errorOf("fn main() {\n\tif (local_invocation_id.x > 3u) {\n\t\treturn;\n\t}\n\tbarrier();\n}\n");
		EXPECT_EQ(message, "'barrier' under lane-dependent control; lanes that skip it deadlock the workgroup");
	}

	TEST(ComputeTest, MemoryBarrierUnderDivergenceIsError) {
		const auto message = errorOf("fn main() {\n\tif (local_invocation_id.x > 3u) {\n\t\tmemory_barrier_shared();\n\t}\n}\n");
		EXPECT_EQ(message, "'memory_barrier_shared' under lane-dependent control; lanes that skip it deadlock the workgroup");
	}

	TEST(ComputeTest, AtomicOnLocalIsRejected) {
		const auto message = errorOf("fn main() {\n\tlet x: u32 = 0u;\n\tlet old = atomic_add(x, 1u);\n}\n");
		EXPECT_EQ(message, "'atomic_add' needs a storage buffer or workgroup place, got local 'x'");
	}

	TEST(ComputeTest, AtomicOnWorkgroupAndStorageIsAccepted) {
		EXPECT_EQ(errorOf("workgroup counter: u32;\nfn main() {\n\tlet old = atomic_add(counter, 1u);\n}\n"), "");
		EXPECT_EQ(errorOf(String { buffer } + "fn main() {\n\tlet old = atomic_add(buf.data[1], 1u);\n}\n"), "");
	}

	TEST(ComputeTest, ImageStoreOnNonImageIsRejected) {
		const auto writable = String { "@input (mode = bind, id = \"img\", class = image, access = write) image2d<rgba16f> img;\n" };
		const auto message = errorOf(writable + "fn main() {\n\tlet c = img;\n\timage_store(c, s32vec2(0, 0), f32vec4(1.0));\n}\n");
		EXPECT_EQ(message, "'image_store' needs a storage image place, got local 'c'");
	}

	TEST(ComputeTest, AtomicThroughParameterIsRejected) {
		const auto message = errorOf("fn bump(x: &mut u32) {\n\tatomic_add(x, 1u);\n}\nfn main() {}\n");
		EXPECT_EQ(message, "'atomic_add' cannot act through parameter 'x'; GLSL atomics need the storage buffer or workgroup variable itself");
	}

	TEST(ComputeTest, AtomicThroughAliasResolvesRoot) {
		EXPECT_EQ(errorOf(String { buffer } + "fn main() {\n\tlet l = &mut buf.data[0];\n\tatomic_add(l, 1u);\n}\n"), "");
		EXPECT_EQ(
			errorOf("fn main() {\n\tlet x: u32 = 0u;\n\tlet l = &mut x;\n\tatomic_add(l, 1u);\n}\n"),
			"'atomic_add' needs a storage buffer or workgroup place, got local 'x'"
		);
	}

	TEST(ComputeTest, AtomicAliasIgnoresShadowingLocal) {
		EXPECT_EQ(errorOf("workgroup counter: u32;\nfn main() {\n\tlet l = &mut counter;\n\t{\n\t\tlet counter: u32 = 0u;\n\t\tatomic_add(l, 1u);\n\t}\n}\n"), "");
	}

	TEST(ComputeTest, StorageImageOutsideComputeIsDiagnostic) {
		const auto message = errorOf(String { image } + "fn main() {}\n", EntryKind::eFragment);
		EXPECT_EQ(message, "storage images require a compute module.");
	}

	TEST(ComputeTest, BarrierInCalleeUnderDivergenceReportsChain) {
		const auto message = errorOf(
			"fn sync() {\n\tbarrier();\n}\nfn main() {\n\tif (local_invocation_id.x > 3u) {\n\t\tsync();\n\t}\n}\n"
		);
		EXPECT_TRUE(message.contains("(via sync)")) << message;
	}

	TEST(ComputeTest, DivergentBreakBeforeBarrierIsError) {
		const auto message = errorOf(
			"fn main() {\n\tfor (let i: u32 = 0u; i < 4u; i += 1u) {\n\t\tif (local_invocation_id.x > 3u) {\n\t\t\tbreak;\n\t\t}\n\t\tbarrier();\n\t}\n}\n"
		);
		EXPECT_TRUE(message.contains("'barrier' under lane-dependent control")) << message;
	}

	TEST(VariantKeyTest, LocalSizeChangesKey) {
		const auto a = parseOk("@compute (local_size = [8, 8, 1])\nfn main() {}\n");
		const auto b = parseOk("@compute (local_size = [16, 8, 1])\nfn main() {}\n");
		EXPECT_NE(contentHash(a), contentHash(b));
		EXPECT_NE(makeVariantKey(a, {}, {}), makeVariantKey(b, {}, {}));
	}

	TEST(FragmentTest, EarlyFragmentTestsEmitted) {
		auto module = parseOk("@fragment (early_fragment_tests = true)\nfn main() {}\n", EntryKind::eFragment);
		EXPECT_TRUE(module.attributes.earlyFragmentTests);
		EXPECT_TRUE(emitGlsl(module).contains("layout(early_fragment_tests) in;"));
	}
}
