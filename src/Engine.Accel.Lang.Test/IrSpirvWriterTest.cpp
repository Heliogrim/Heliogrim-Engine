#include "pch.h"

/**/

#include <spirv/unified1/spirv.hpp11>

/**/

import Engine.Accel.Lang;
import std;

using namespace hg;
using namespace hg::engine::accel::lang::ir::spirv;

namespace {
	Vector<u32> compute(const bool withReturn_) {
		SpirvWriter w {};
		const auto voidT = w.type(spv::Op::OpTypeVoid, {});
		const std::array<u32, 1> ret { voidT };
		const auto fnT = w.type(spv::Op::OpTypeFunction, ret);
		const auto fn = w.id();
		const auto label = w.id();

		const std::array<u32, 1> cap { static_cast<u32>(spv::Capability::Shader) };
		w.capability(spv::Op::OpCapability, cap);
		const std::array<u32, 2> mm { static_cast<u32>(spv::AddressingModel::Logical), static_cast<u32>(spv::MemoryModel::GLSL450) };
		w.memoryModel(spv::Op::OpMemoryModel, mm);

		Vector<u32> ep { static_cast<u32>(spv::ExecutionModel::GLCompute), fn };
		SpirvWriter::string(ep, "main");
		w.entryPoint(spv::Op::OpEntryPoint, ep);
		const std::array<u32, 5> em { fn, static_cast<u32>(spv::ExecutionMode::LocalSize), 1uL, 1uL, 1uL };
		w.executionMode(spv::Op::OpExecutionMode, em);

		const std::array<u32, 4> head { voidT, fn, 0uL, fnT };
		w.function(spv::Op::OpFunction, head);
		const std::array<u32, 1> lbl { label };
		w.function(spv::Op::OpLabel, lbl);
		if (withReturn_) {
			w.function(spv::Op::OpReturn, {});
		}
		w.function(spv::Op::OpFunctionEnd, {});
		return w.finish();
	}
}

TEST(IrSpirvWriterTest, EmptyComputeValidates) {
	const auto words = compute(true);
	const auto diags = validate(words, "empty");
	EXPECT_EQ(diags.size(), 0uz) << (diags.empty() ? String {} : diags.front().message());
}

TEST(IrSpirvWriterTest, TypesDeduplicate) {
	SpirvWriter w {};
	const std::array<u32, 2> i32 { 32uL, 0uL };
	EXPECT_EQ(w.type(spv::Op::OpTypeInt, i32), w.type(spv::Op::OpTypeInt, i32));
	EXPECT_NE(w.type(spv::Op::OpTypeInt, i32, true), w.type(spv::Op::OpTypeInt, i32));
}

TEST(IrSpirvWriterTest, ConstantsDeduplicate) {
	SpirvWriter w {};
	const std::array<u32, 2> i32 { 32uL, 0uL };
	const std::array<u32, 2> i64 { 64uL, 0uL };
	const auto a = w.type(spv::Op::OpTypeInt, i32);
	const auto b = w.type(spv::Op::OpTypeInt, i64);
	const std::array<u32, 1> one { 1uL };
	EXPECT_EQ(w.constant(spv::Op::OpConstant, a, one), w.constant(spv::Op::OpConstant, a, one));
	EXPECT_NE(w.constant(spv::Op::OpConstant, a, one), w.constant(spv::Op::OpConstant, b, one));
	EXPECT_NE(w.constant(spv::Op::OpConstant, a, one, true), w.constant(spv::Op::OpConstant, a, one));
}

TEST(IrSpirvWriterTest, StringsPackLittleEndian) {
	Vector<u32> out {};
	SpirvWriter::string(out, "main");
	ASSERT_EQ(out.size(), 2uz);
	EXPECT_EQ(out[0], 0x6E69616DuL);
	EXPECT_EQ(out[1], 0uL);
}

TEST(IrSpirvWriterTest, InvalidModuleReports) {
	const auto words = compute(false);
	const auto diags = validate(words, "broken");
	ASSERT_EQ(diags.size(), 1uz);
	EXPECT_NE(diags.front().message().find("broken"), String::npos);
	EXPECT_NE(diags.front().message().find("Function end"), String::npos) << diags.front().message();
	EXPECT_EQ(diags.front().code()->key, "internal-spirv-invalid");
}

TEST(IrSpirvWriterTest, OverlongInstructionIsReportedNotWritten) {
	SpirvWriter w {};
	EXPECT_FALSE(w.overlong());

	const Vector<u32> operands(0xFFFFuz, 0uL);
	w.function(spv::Op::OpNop, operands);
	EXPECT_TRUE(w.overlong());
	EXPECT_EQ(w.finish().size(), 5uz);
}
