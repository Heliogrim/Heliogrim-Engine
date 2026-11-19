#pragma once

#include <Engine.Common/__macro.hpp>

#include "../Diagnostic.hpp"
#include "../Layout.hpp"
#include "../Module.hpp"
#include "../Tier.hpp"
#include "SpirvWriter.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::accel::lang::ir::spirv {
	using Ops = Vector<u32>;

	template <typename Enum_>
	[[nodiscard]] constexpr u32 w32(Enum_ value_) noexcept {
		return static_cast<u32>(value_);
	}

	/* a float truncates into an integer of `bits_` (0 = 64); NaN and out-of-range conversions are undefined, so they do not fold */
	[[nodiscard]] inline bool fitsInteger(const f64 value_, const ScalarKind kind_, const u32 bits_) noexcept {
		const auto width = bits_ == 0uL || bits_ > 64uL ? 64 : static_cast<int>(bits_);
		return kind_ == ScalarKind::eInt ?
			value_ >= -std::ldexp(1.0, width - 1) && value_ < std::ldexp(1.0, width - 1) :
			value_ > -1.0 && value_ < std::ldexp(1.0, width);
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
	public:
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

	public:
		Impl(ref<const Module> mod_, ref<const Layout> layout_, const Tier tier_) :
			mod(mod_),
			layout(layout_),
			tier(tier_) {}

	public:
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
		/* builtin variables declared so far, by source name */
		std::map<String, Place> builtinVars {};

	public:
		/* SpirvTypes.cpp */
		[[nodiscard]] u32 type(const TypeRef type_);

		[[nodiscard]] u32 laidOut(const TypeRef type_, const bool std140_);

		/* Block struct of a buffer/push binding, member offsets shifted by `bias_` (push range offset) */
		[[nodiscard]] u32 block(const TypeRef type_, const bool std140_, const u32 bias_);

		[[nodiscard]] u32 pointer(const spv::StorageClass storage_, const u32 pointee_);

		[[nodiscard]] u32 scalar(const ScalarKind kind_, const u32 bits_);

		[[nodiscard]] u32 voidType();

		[[nodiscard]] u32 boolType();

		[[nodiscard]] u32 u32Type();

		[[nodiscard]] u32 u32Constant(const u32 value_);

		/* OpConstant/True/False of a scalar type; 0 after a diagnostic */
		[[nodiscard]] u32 literal(const TypeRef type_, ref<const LiteralValue> literal_, const SourceLoc loc_);

		[[nodiscard]] u32 literal(ref<const TypeDesc> scalar_, ref<const LiteralValue> literal_, const SourceLoc loc_);

		/* the OpConstant / OpSpecConstant value words of a non-bool scalar; false after a diagnostic */
		[[nodiscard]] bool literalWords(const TypeRef type_, ref<const LiteralValue> literal_, ref<Ops> out_, const SourceLoc loc_);

		[[nodiscard]] bool literalWords(ref<const TypeDesc> scalar_, ref<const LiteralValue> literal_, ref<Ops> out_, const SourceLoc loc_);

		void require(const spv::Capability capability_);

		void fail(ref<const DiagnosticCodeInfo> code_, SourceLoc loc_, String message_);

		/* OpName from Tier::eDev on; nothing for an empty name */
		void nameOf(const u32 id_, const StringView name_);

		/* SpirvInterface.cpp */
		/* user errors of the declarations the earlier passes leave to the emitter: bindings, structs, `main`'s signature */
		void checkDeclarations();

		void reach();

		void declareGlobals();

		void declareConstants();

		void declareFunctions();

		void entryPoint();

		/* SpirvFunctions.cpp: bodies of every reached function */
		void bodies();

		/* SpirvFeatures.cpp: capabilities and execution modes implied by the emitted module; diagnoses what is outside the profile */
		void features();

		/* function-section instruction emission of the body being built; nothing is emitted once the block has ended */
		[[nodiscard]] u32 op(const spv::Op op_, const u32 resultType_, Ops operands_);

		void code(const spv::Op op_, Ops operands_);

		/* the only way a block ends (branch, return, kill, unreachable) */
		void terminate(const spv::Op op_, Ops operands_);

		void label(const u32 label_);

		/* OpVariable Function in the first block */
		[[nodiscard]] u32 local(const TypeRef type_);

		[[nodiscard]] u32 value(const ExprRef expr_);

		[[nodiscard]] Place place(const ExprRef expr_);

		[[nodiscard]] u32 load(ref<const Place> place_);

		void store(ref<const Place> place_, const u32 value_);

		/* Task 7 hooks: builtin calls / variables by `Expr::name` (only `bitcast` here) */
		[[nodiscard]] u32 builtin(const ExprRef expr_);

		[[nodiscard]] Place builtinPlace(const ExprRef expr_);

	private:
		struct Instr {
			spv::Op op;
			Ops operands;
		};

		struct Loop {
			u32 merge;
			u32 cont;
		};

	private:
		/* body state, reset per function */
		Vector<Instr> locals {};
		Vector<Instr> body {};
		u32 current = 0uL;
		bool ended = false;
		Vector<Loop> loops {};
		Vector<std::pair<String, Place>> scope {};

	private:
		[[nodiscard]] u32 image(
			ref<const TypeDesc> desc_,
			const ScalarKind sampled_,
			const u32 sampledMode_,
			const spv::ImageFormat format_
		);

		[[nodiscard]] u32 structure(const TypeRef type_, const bool std140_, const u32 bias_, const bool block_);

		void memberLayout(const u32 struct_, const u32 member_, const u32 offset_, const TypeRef type_, const bool std140_);

		void names(const u32 id_, ref<const TypeDesc> desc_);

		[[nodiscard]] u32 vertexCount(const bool input_);

		[[nodiscard]] u32 glslStd();

		void statement(const StmtRef stmt_);

		void loop(const StmtRef init_, const ExprRef cond_, const StmtRef body_, const StmtRef step_);

		[[nodiscard]] u32 pointerOf(ref<const Place> place_);

		[[nodiscard]] u32 pointeeOf(ref<const Place> place_);

		[[nodiscard]] u32 fromLaid(const TypeRef type_, const u32 laid_, const u32 value_, const bool std140_);

		[[nodiscard]] u32 toLaid(const TypeRef type_, const u32 laid_, const u32 value_, const bool std140_);

		[[nodiscard]] u32 shortCircuit(ref<const Expr> expr_);

		[[nodiscard]] u32 ternary(ref<const Expr> expr_);

		[[nodiscard]] u32 binary(
			const Op op_,
			const TypeRef result_,
			const TypeRef lt_,
			const u32 l_,
			const TypeRef rt_,
			const u32 r_,
			const SourceLoc loc_
		);

		[[nodiscard]] u32 construct(ref<const Expr> expr_);

		[[nodiscard]] u32 convert(const TypeRef from_, const u32 value_, const ScalarKind kind_, const u32 bits_, const SourceLoc loc_);

		[[nodiscard]] u32 call(ref<const Expr> expr_);

		[[nodiscard]] u32 field(const ExprRef expr_);

		[[nodiscard]] u32 number(const ScalarKind kind_, const u32 bits_, const u32 n_, const bool one_);

		[[nodiscard]] u32 shaped(const ScalarKind kind_, const u32 bits_, const u32 n_);
	};
}
