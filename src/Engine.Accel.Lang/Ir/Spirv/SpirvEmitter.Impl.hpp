#pragma once

#include <map>
#include <set>
#include <utility>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/String.hpp>
#include <Engine.Common/Types.hpp>
#include <Engine.Common/Wrapper.hpp>

#include "SpirvWriter.hpp"
#include "../Diagnostic.hpp"
#include "../Layout.hpp"
#include "../Module.hpp"
#include "../Tier.hpp"

namespace hg::engine::accel::lang::ir::spirv {
	using Ops = Vector<u32>;

	template <typename Enum_>
	[[nodiscard]] constexpr u32 w32(Enum_ value_) noexcept {
		return static_cast<u32>(value_);
	}

	/**
	 * A module-scope variable of a binding. With `explicitLayout` the pointee is a laid-out block: members use
	 * `laidOut` types, so bool members are stored as uint (bool vectors as uint vectors) and must be converted on load/store.
	 */
	struct GlobalVar {
		/* 0 = not declared (unreached or implicit) */
		u32 id = 0uL;
		spv::StorageClass storage = spv::StorageClass::Private;
		u32 pointee = 0uL;
		bool explicitLayout = false;
		bool std140 = false;
		/* non-struct block type wrapped as member 0 of the block struct */
		bool wrapped = false;
		/* per-vertex forward variable: pointee is `OpTypeArray <type> arrayedCount` */
		u32 arrayedCount = 0uL;
	};

	struct FunctionInfo {
		u32 id = 0uL;
		u32 type = 0uL;
		u32 ret = 0uL;
		/* pre-allocated OpFunctionParameter result ids and their types (ref params: Function pointers) */
		Vector<u32> params;
		Vector<u32> paramTypes;
	};

	/**
	 * Emitter state shared by SpirvTypes.cpp, SpirvInterface.cpp and SpirvFunctions.cpp; every table is indexed like the module's.
	 */
	struct Impl {
		Impl(ref<const Module> mod_, ref<const Layout> layout_, Tier tier_) :
			mod(mod_),
			layout(layout_),
			tier(tier_) {}

		ref<const Module> mod;
		ref<const Layout> layout;
		Tier tier;

		SpirvWriter w {};
		Diagnostics diagnostics {};
		std::set<spv::Capability> capabilities {};

		/* value type ids by TypeRef (0 = not yet declared) */
		Vector<u32> valueTypes {};
		/* explicitly laid-out type ids by (TypeRef index, std140) */
		std::map<std::pair<u32, bool>, u32> laidOutTypes {};

		/* by BindingRef */
		Vector<GlobalVar> globals {};
		/* by ConstantRef (0 = unreached) */
		Vector<u32> constants {};
		/* by FunctionRef (id 0 = unreached) */
		Vector<FunctionInfo> functions {};

		Vector<bool> reachedFunctions {};
		Vector<bool> reachedBindings {};
		Vector<bool> reachedConstants {};
		/* FunctionRef index of `main` */
		u32 entry = ~0uL;
		/* OpEntryPoint interface: every declared global variable */
		Vector<u32> interface {};
		/* GLSL.std.450 import (0 = not yet imported) */
		u32 glsl450 = 0uL;
		/* OpString id per `Module::sources` entry (Tier::eDev on) */
		Vector<u32> debugFiles {};
		std::set<String> extensions {};

		/* SpirvTypes.cpp */
		[[nodiscard]] u32 type(TypeRef type_);
		[[nodiscard]] u32 laidOut(TypeRef type_, bool std140_);
		/* Block struct of a buffer/push binding, member offsets shifted by `bias_` (push range offset) */
		[[nodiscard]] u32 block(TypeRef type_, bool std140_, u32 bias_);
		[[nodiscard]] u32 pointer(spv::StorageClass storage_, u32 pointee_);
		[[nodiscard]] u32 scalar(ScalarKind kind_, u32 bits_);
		[[nodiscard]] u32 voidType();
		[[nodiscard]] u32 boolType();
		[[nodiscard]] u32 u32Type();
		[[nodiscard]] u32 u32Constant(u32 value_);
		/* OpConstant/True/False of a scalar type; 0 after a diagnostic */
		[[nodiscard]] u32 literal(TypeRef type_, ref<const LiteralValue> literal_);
		/* the OpConstant / OpSpecConstant value words of a non-bool scalar; false after a diagnostic */
		[[nodiscard]] bool literalWords(TypeRef type_, ref<const LiteralValue> literal_, ref<Ops> out_);

		void require(spv::Capability capability_);
		void fail(ref<const DiagnosticCodeInfo> code_, SourceLoc loc_, String message_);
		/* OpName from Tier::eDev on; nothing for an empty name */
		void nameOf(u32 id_, StringView name_);

		/* SpirvInterface.cpp */
		void reach();
		void declareGlobals();
		void declareConstants();
		void declareFunctions();
		void entryPoint();

		/* SpirvFunctions.cpp: bodies of every reached function */
		void bodies();

		/**
		 * An addressable location: `base` (a variable or pointer parameter) plus OpAccessChain indices (ids), not yet
		 * materialised. `base == 0` = not addressable. `basePointee` is the base's pointee, used while `indices` is empty.
		 */
		struct Place {
			u32 base = 0uL;
			Ops indices {};
			spv::StorageClass storage = spv::StorageClass::Function;
			bool explicitLayout = false;
			bool std140 = false;
			u32 basePointee = 0uL;
			TypeRef type {};
		};

		/* builtin variables declared so far, by source name */
		std::map<String, Place> builtinVars {};

		/* SpirvFeatures.cpp: capabilities and execution modes implied by the emitted module; diagnoses what is outside the profile */
		void features();

		/* function-section instruction emission of the body being built; nothing is emitted once the block has ended */
		[[nodiscard]] u32 op(spv::Op op_, u32 resultType_, Ops operands_);
		void code(spv::Op op_, Ops operands_);
		/* the only way a block ends (branch, return, kill, unreachable) */
		void terminate(spv::Op op_, Ops operands_);
		void label(u32 label_);
		/* OpVariable Function in the first block */
		[[nodiscard]] u32 local(TypeRef type_);

		[[nodiscard]] u32 value(ExprRef expr_);
		[[nodiscard]] Place place(ExprRef expr_);
		[[nodiscard]] u32 load(ref<const Place> place_);
		void store(ref<const Place> place_, u32 value_);

		/* Task 7 hooks: builtin calls / variables by `Expr::name` (only `bitcast` here) */
		[[nodiscard]] u32 builtin(ExprRef expr_);
		[[nodiscard]] Place builtinPlace(ExprRef expr_);

	private:
		[[nodiscard]] u32 image(ref<const TypeDesc> desc_, ScalarKind sampled_, u32 sampledMode_, spv::ImageFormat format_);
		[[nodiscard]] u32 structure(TypeRef type_, bool std140_, u32 bias_, bool block_);
		void memberLayout(u32 struct_, u32 member_, u32 offset_, TypeRef type_, bool std140_);
		void names(u32 id_, ref<const TypeDesc> desc_);
		[[nodiscard]] u32 vertexCount(bool input_);
		[[nodiscard]] u32 glslStd();

		struct Instr {
			spv::Op op;
			Ops operands;
		};

		struct Loop {
			u32 merge;
			u32 cont;
		};

		/* body state, reset per function */
		Vector<Instr> locals {};
		Vector<Instr> body {};
		u32 current = 0uL;
		bool ended = false;
		Vector<Loop> loops {};
		Vector<std::pair<String, Place>> scope {};

		void statement(StmtRef stmt_);
		void loop(StmtRef init_, ExprRef cond_, StmtRef body_, StmtRef step_);
		[[nodiscard]] u32 pointerOf(ref<const Place> place_);
		[[nodiscard]] u32 pointeeOf(ref<const Place> place_);
		[[nodiscard]] u32 fromLaid(TypeRef type_, u32 laid_, u32 value_, bool std140_);
		[[nodiscard]] u32 toLaid(TypeRef type_, u32 laid_, u32 value_, bool std140_);
		[[nodiscard]] u32 shortCircuit(ref<const Expr> expr_);
		[[nodiscard]] u32 ternary(ref<const Expr> expr_);
		[[nodiscard]] u32 binary(Op op_, TypeRef result_, TypeRef lt_, u32 l_, TypeRef rt_, u32 r_, SourceLoc loc_);
		[[nodiscard]] u32 construct(ref<const Expr> expr_);
		[[nodiscard]] u32 convert(TypeRef from_, u32 value_, ScalarKind kind_, u32 bits_, SourceLoc loc_);
		[[nodiscard]] u32 call(ref<const Expr> expr_);
		[[nodiscard]] u32 field(ExprRef expr_);
		[[nodiscard]] u32 number(ScalarKind kind_, u32 bits_, u32 n_, bool one_);
		[[nodiscard]] u32 shaped(ScalarKind kind_, u32 bits_, u32 n_);
	};
}
