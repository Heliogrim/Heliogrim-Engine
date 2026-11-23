#include "pch.h"

/**/

#include <spirv/unified1/spirv.hpp11>

/**/

#include "IrSpirvText.hpp"

import Engine.Accel.Lang;
import std;

using namespace hg::engine::accel::lang::ir;
using namespace hg::engine::accel::lang;
using namespace hg;

namespace {
	using spirv::SpirvWriter;

	/**
	 * `struct S { u32 a; u32 b; }; S arr[4]` in Function storage, the callee stores 3 into `arr[1].a`:
	 * through a pointer to the element field (`wholeRoot_ = false`) or through the whole variable plus an index.
	 */
	Vector<u32> localPointer(const bool wholeRoot_) {
		SpirvWriter w {};
		const auto fnClass = static_cast<u32>(spv::StorageClass::Function);
		const auto voidT = w.type(spv::Op::OpTypeVoid, {});
		const std::array<u32, 2> intOps { 32uL, 0uL };
		const auto u32T = w.type(spv::Op::OpTypeInt, intOps);
		const std::array<u32, 2> fields { u32T, u32T };
		const auto structT = w.type(spv::Op::OpTypeStruct, fields);
		const std::array<u32, 1> four { 4uL }, zero { 0uL }, one { 1uL }, three { 3uL };
		const auto c4 = w.constant(spv::Op::OpConstant, u32T, four);
		const auto c0 = w.constant(spv::Op::OpConstant, u32T, zero);
		const auto c1 = w.constant(spv::Op::OpConstant, u32T, one);
		const auto c3 = w.constant(spv::Op::OpConstant, u32T, three);
		const std::array<u32, 2> arrOps { structT, c4 };
		const auto arrT = w.type(spv::Op::OpTypeArray, arrOps);
		const std::array<u32, 2> pArrOps { fnClass, arrT };
		const auto pArr = w.type(spv::Op::OpTypePointer, pArrOps);
		const std::array<u32, 2> pU32Ops { fnClass, u32T };
		const auto pU32 = w.type(spv::Op::OpTypePointer, pU32Ops);

		const std::array<u32, 1> mainOps { voidT };
		const auto mainT = w.type(spv::Op::OpTypeFunction, mainOps);
		const std::array<u32, 2> pathOps { voidT, pU32 };
		const std::array<u32, 3> rootOps { voidT, pArr, u32T };
		const auto calleeT = wholeRoot_ ? w.type(spv::Op::OpTypeFunction, rootOps) : w.type(spv::Op::OpTypeFunction, pathOps);

		const auto mainFn = w.id();
		const auto callee = w.id();

		const std::array<u32, 1> cap { static_cast<u32>(spv::Capability::Shader) };
		w.capability(spv::Op::OpCapability, cap);
		const std::array<u32, 2> mm { static_cast<u32>(spv::AddressingModel::Logical), static_cast<u32>(spv::MemoryModel::GLSL450) };
		w.memoryModel(spv::Op::OpMemoryModel, mm);
		Vector<u32> ep { static_cast<u32>(spv::ExecutionModel::GLCompute), mainFn };
		SpirvWriter::string(ep, "main");
		w.entryPoint(spv::Op::OpEntryPoint, ep);
		const std::array<u32, 5> em { mainFn, static_cast<u32>(spv::ExecutionMode::LocalSize), 1uL, 1uL, 1uL };
		w.executionMode(spv::Op::OpExecutionMode, em);

		// callee
		const std::array<u32, 4> calleeHead { voidT, callee, 0uL, calleeT };
		w.function(spv::Op::OpFunction, calleeHead);
		const auto param = w.id();
		const std::array<u32, 2> paramOps { wholeRoot_ ? pArr : pU32, param };
		w.function(spv::Op::OpFunctionParameter, paramOps);
		const auto index = w.id();
		if (wholeRoot_) {
			const std::array<u32, 2> indexOps { u32T, index };
			w.function(spv::Op::OpFunctionParameter, indexOps);
		}
		const std::array<u32, 1> calleeLabel { w.id() };
		w.function(spv::Op::OpLabel, calleeLabel);
		auto target = param;
		if (wholeRoot_) {
			target = w.id();
			const std::array<u32, 5> chain { pU32, target, param, index, c0 };
			w.function(spv::Op::OpAccessChain, chain);
		}
		const std::array<u32, 2> store { target, c3 };
		w.function(spv::Op::OpStore, store);
		w.function(spv::Op::OpReturn, {});
		w.function(spv::Op::OpFunctionEnd, {});

		// main
		const std::array<u32, 4> mainHead { voidT, mainFn, 0uL, mainT };
		w.function(spv::Op::OpFunction, mainHead);
		const std::array<u32, 1> mainLabel { w.id() };
		w.function(spv::Op::OpLabel, mainLabel);
		const auto var = w.id();
		const std::array<u32, 3> varOps { pArr, var, fnClass };
		w.function(spv::Op::OpVariable, varOps);
		const auto call = w.id();
		if (wholeRoot_) {
			const std::array<u32, 5> callOps { voidT, call, callee, var, c1 };
			w.function(spv::Op::OpFunctionCall, callOps);
		} else {
			const auto field = w.id();
			const std::array<u32, 5> chain { pU32, field, var, c1, c0 };
			w.function(spv::Op::OpAccessChain, chain);
			const std::array<u32, 4> callOps { voidT, call, callee, field };
			w.function(spv::Op::OpFunctionCall, callOps);
		}
		w.function(spv::Op::OpReturn, {});
		w.function(spv::Op::OpFunctionEnd, {});
		return w.finish();
	}

	[[nodiscard]] Opt<String> noIncludes(StringView) {
		return None;
	}

	constexpr auto globals =
		"@input (mode = bind, id = \"counts\", class = storage, access = read_write) block counts { u32[] data; };\n"
		"@input (mode = bind, id = \"other\", class = storage, access = read_write) block other { u32[] data; };\n"
		"fn bump(v: &mut u32) { v += 1u; }\n";

	[[nodiscard]] std::expected<Module, Diagnostics> lowered(StringView main_) {
		auto parsed = parseModule(
			"M",
			String { globals } + "fn main() {\nlet i: u32 = local_invocation_index;\n" + String { main_ } + "\n}\n",
			EntryKind::eCompute,
			ModuleResolver { noIncludes }
		);
		if (not parsed.has_value()) {
			return parsed;
		}
		return lowerReferences(*parsed);
	}

	[[nodiscard]] String emitGlsl(ref<const Module> mod_) {
		const auto result = AccelLang::spirvOf(mod_);
		if (not result.diagnostics.empty()) {
			return "error: " + result.diagnostics.front().message();
		}
		return result.text;
	}

	/* specialisations of source function `origin_` */
	[[nodiscard]] Vector<FunctionDecl> specsOf(ref<const Module> mod_, StringView origin_) {
		Vector<FunctionDecl> out {};
		for (const auto& fn : mod_.functions) {
			if (fn.name.empty() && fn.origin == origin_) {
				out.emplace_back(fn);
			}
		}
		return out;
	}
}

namespace AccelLang {
	TEST(IrReferenceTest, LocalPathPointerIsRejected) {
		// R3 (a): a pointer to a field of a Function array element passed to a call
		const auto diags = spirv::validate(localPointer(false), "local-path");
		ASSERT_EQ(diags.size(), 1uz);
		EXPECT_EQ(diags.front().code()->key, "internal-spirv-invalid") << diags.front().message();
		EXPECT_TRUE(diags.front().message().contains("memory object declaration")) << diags.front().message();
	}

	TEST(IrReferenceTest, LocalRootPointerIsLegal) {
		// R3 (b): the whole Function variable plus a u32 index
		const auto diags = spirv::validate(localPointer(true), "local-root");
		EXPECT_TRUE(diags.empty()) << diags.front().message();
	}

	TEST(IrReferenceTest, GlobalRootSpecialisedPerRoot) {
		auto mod = lowered("bump(counts.data[i]); bump(other.data[0u]);");
		ASSERT_TRUE(mod.has_value()) << mod.error().front().message();
		// `bump` is function 0 of the source module and is dropped
		EXPECT_FALSE(std::ranges::any_of(mod->functions, [](const auto& fn_) { return fn_.name == "bump"; }));
		const auto specs = specsOf(*mod, "bump");
		ASSERT_EQ(specs.size(), 2uz);
		ASSERT_EQ(specs[0].params.size(), 1uz);
		EXPECT_EQ(specs[0].params[0].name, "v$i0");
		EXPECT_EQ(specs[0].params[0].kind, ParamKind::eValue);
		EXPECT_TRUE(specs[1].params.empty());
		EXPECT_FALSE(std::ranges::any_of(mod->exprs, [](const auto& e_) { return e_.kind == ExprKind::eRange; }));
	}

	TEST(IrReferenceTest, WriteVisibleThroughGlobalName) {
		auto mod = lowered("bump(counts.data[i]); bump(other.data[0u]);");
		ASSERT_TRUE(mod.has_value()) << mod.error().front().message();
		const auto glsl = emitGlsl(*mod);
		EXPECT_FALSE(glsl.starts_with("error:")) << glsl;
		EXPECT_TRUE(glsl.contains("OpAccessChain")) << glsl;
		EXPECT_FALSE(glsl.contains("\"bump\"")) << glsl;
	}

	TEST(IrReferenceTest, SameRootCallsShareSpecialisation) {
		auto mod = lowered("bump(counts.data[i]); bump(counts.data[i + 1u]); bump(counts.data[2u]); bump(counts.data[2u]);");
		ASSERT_TRUE(mod.has_value()) << mod.error().front().message();
		EXPECT_EQ(specsOf(*mod, "bump").size(), 2uz);
	}

	TEST(IrReferenceTest, LetAliasBecomesPlace) {
		auto mod = lowered("let r = &mut counts.data[i]; i = 3u; r = 7u;");
		ASSERT_TRUE(mod.has_value()) << mod.error().front().message();
		const auto glsl = emitGlsl(*mod);
		EXPECT_FALSE(glsl.starts_with("error:")) << glsl;
		EXPECT_TRUE(glsl.contains("OpAccessChain")) << glsl;
	}

	TEST(IrReferenceTest, LocalRootKeepsReference) {
		auto parsed = parseModule(
			"M",
			"struct P { u32 colour; u32 w; }\n"
			"fn paint(c: &mut u32) { c = 3u; }\n"
			"fn main() {\nlet i: u32 = local_invocation_index;\nlet arr: P[4];\npaint(arr[i].colour);\n}\n",
			EntryKind::eCompute,
			ModuleResolver { noIncludes }
		);
		ASSERT_TRUE(parsed.has_value()) << parsed.error().front().message();
		auto mod = lowerReferences(*parsed);
		ASSERT_TRUE(mod.has_value()) << mod.error().front().message();
		const auto specs = specsOf(*mod, "paint");
		ASSERT_EQ(specs.size(), 1uz);
		ASSERT_EQ(specs[0].params.size(), 2uz);
		EXPECT_EQ(specs[0].params[0].name, "c");
		EXPECT_EQ(specs[0].params[0].kind, ParamKind::eRefMut);
		ASSERT_LT(specs[0].params[0].type.index, mod->types.size());
		EXPECT_EQ(mod->types[specs[0].params[0].type.index].kind, TypeKind::eArray);
		EXPECT_EQ(specs[0].params[1].name, "c$i0");
		const auto glsl = emitGlsl(*mod);
		EXPECT_FALSE(glsl.starts_with("error:")) << glsl;
		EXPECT_TRUE(glsl.contains("OpFunctionCall")) << glsl;
	}

	TEST(IrReferenceTest, IndexTypeDoesNotSplitSpecialisation) {
		auto mod = lowered("let j: s32 = 1;\nbump(counts.data[j]); bump(counts.data[i]); bump(counts.data[0]); bump(counts.data[0u]);");
		ASSERT_TRUE(mod.has_value()) << mod.error().front().message();
		// one for `[*]` (s32 and u32 index), one for `[0]`/`[0u]`; `loc` stays on `bump`
		const auto specs = specsOf(*mod, "bump");
		ASSERT_EQ(specs.size(), 2uz);
		EXPECT_EQ(specs[0].loc.line, 3uL);
		const auto glsl = emitGlsl(*mod);
		EXPECT_FALSE(glsl.starts_with("error:")) << glsl;
		EXPECT_TRUE(glsl.contains("OpFunctionCall")) << glsl;
	}

	TEST(IrReferenceTest, NestedGlobalRootForwardsIndex) {
		auto parsed = parseModule(
			"M",
			"@input (mode = bind, id = \"counts\", class = storage, access = read_write) block counts { u32[] data; };\n"
			"fn b(w: &mut u32) { w += 1u; }\n"
			"fn a(v: &mut u32) { b(v); }\n"
			"fn main() {\nlet i: u32 = local_invocation_index;\na(counts.data[i]);\n}\n",
			EntryKind::eCompute,
			ModuleResolver { noIncludes }
		);
		ASSERT_TRUE(parsed.has_value()) << parsed.error().front().message();
		auto mod = lowerReferences(*parsed);
		ASSERT_TRUE(mod.has_value()) << mod.error().front().message();
		ASSERT_EQ(specsOf(*mod, "a").size(), 1uz);
		ASSERT_EQ(specsOf(*mod, "b").size(), 1uz);
		const auto glsl = emitGlsl(*mod);
		EXPECT_FALSE(glsl.starts_with("error:")) << glsl;
		EXPECT_TRUE(glsl.contains("OpFunctionCall")) << glsl;
	}

	TEST(IrReferenceTest, NestedLocalRootConcatenatesPath) {
		auto parsed = parseModule(
			"M",
			"struct P { u32 colour; u32 w; }\n"
			"fn b(w: &mut u32) { w = 3u; }\n"
			"fn a(v: &mut P) { b(v.colour); }\n"
			"fn main() {\nlet i: u32 = local_invocation_index;\nlet arr: P[4];\na(arr[i]);\n}\n",
			EntryKind::eCompute,
			ModuleResolver { noIncludes }
		);
		ASSERT_TRUE(parsed.has_value()) << parsed.error().front().message();
		auto mod = lowerReferences(*parsed);
		ASSERT_TRUE(mod.has_value()) << mod.error().front().message();
		const auto specs = specsOf(*mod, "b");
		ASSERT_EQ(specs.size(), 1uz);
		ASSERT_EQ(specs[0].params.size(), 2uz);
		ASSERT_LT(specs[0].params[0].type.index, mod->types.size());
		EXPECT_EQ(mod->types[specs[0].params[0].type.index].kind, TypeKind::eArray);
		const auto glsl = emitGlsl(*mod);
		EXPECT_FALSE(glsl.starts_with("error:")) << glsl;
		EXPECT_TRUE(glsl.contains("OpFunctionCall")) << glsl;
	}
}
