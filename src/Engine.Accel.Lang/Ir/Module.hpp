#pragma once

#include <variant>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/Optional.hpp>
#include <Engine.Common/String.hpp>
#include <Engine.Common/Types.hpp>

#include "Condition.hpp"

namespace hg::engine::accel::lang::ir {
	enum class EntryKind : u8 {
		eVertex,
		eTessCtrl,
		eTessEval,
		eGeometry,
		eFragment,
		eCompute
	};

	enum class BindMode : u8 {
		eForward,
		eBind
	};

	enum class BindClass : u8 {
		eNone,
		eUniformBuffer,
		eStorageBuffer,
		eTexture,
		eSampler,
		ePushConstant
	};

	struct TypeRef {
		u32 index = ~0uL;

		[[nodiscard]] bool operator==(const TypeRef&) const noexcept = default;
	};

	/* Arena handles; index-based so arenas can be copied and appended (refs remapped by offset) */
	struct ExprRef {
		u32 index = ~0uL;
	};

	struct StmtRef {
		u32 index = ~0uL;
	};

	struct BindingRef {
		u32 index = ~0uL;
	};

	struct FunctionRef {
		u32 index = ~0uL;
	};

	struct BodyRef {
		StmtRef root;
	};

	enum class ExprKind : u8 {
		eLiteral,
		eNameRef,
		eCall,
		eField,
		eIndex,
		eUnary,
		eBinary,
		eConstruct
	};

	/**
	 * Flat expression node; only the members relevant to `kind` are meaningful.
	 */
	struct Expr {
		ExprKind kind = ExprKind::eLiteral;

		/* literal: source text; nameref/call: referenced name; field: member name; unary/binary: operator */
		String name;

		/* nameref: resolved binding; call: resolved function; ~0 = unresolved (then matched by name).
		 * Typed so arena merging can remap them like the other refs. */
		BindingRef binding;
		FunctionRef function;

		/* nameref resolved to a local or parameter: not a binding use, `binding` stays absent */
		bool local = false;

		/* construct */
		TypeRef type;

		/* call/construct: arguments; field: [base]; index: [base, index]; unary: [operand]; binary: [lhs, rhs] */
		Vector<ExprRef> operands;
	};

	enum class StmtKind : u8 {
		eDeclareLocal,
		eAssign,
		eExpr,
		eReturn,
		eBlock,
		eWhen
	};

	/**
	 * Flat statement node; only the members relevant to `kind` are meaningful.
	 */
	struct Stmt {
		StmtKind kind = StmtKind::eExpr;

		/* declare-local */
		String name;
		TypeRef type;

		/* declare-local: init (optional), assign: lhs, expr/return: value (optional for return) */
		ExprRef lhs;
		/* assign: rhs */
		ExprRef rhs;

		/* block: children; when: [then, else?] */
		Vector<StmtRef> children;
		Condition when;
	};

	enum class TypeKind : u8 {
		eScalar,
		eVector,
		eMatrix,
		eArray,
		eStruct,
		eTexture,
		eSampler
	};

	enum class ScalarKind : u8 {
		eBool,
		eInt,
		eUInt,
		eFloat
	};

	enum class TextureDim : u8 {
		e1D,
		e2D,
		e3D,
		eCube
	};

	struct FieldDesc {
		String name;
		TypeRef type;
	};

	/**
	 * Flat type description; only the members relevant to `kind` are meaningful.
	 */
	struct TypeDesc {
		TypeKind kind = TypeKind::eScalar;
		ScalarKind scalar = ScalarKind::eFloat;
		/* scalar/vector/matrix: component width in bits (u8 vs u32) */
		u32 bits = 32uL;

		/* vector: n = components; matrix: n = columns, m = rows */
		u32 n = 1uL;
		u32 m = 1uL;

		/* array: element type and count (0 = unsized) */
		TypeRef element;
		u32 count = 0uL;

		/* struct */
		String name;
		Vector<FieldDesc> fields;

		/* texture */
		TextureDim dim = TextureDim::e2D;
	};

	struct BindingDecl {
		String id;
		TypeRef type;
		BindMode mode = BindMode::eForward;
		BindClass bindClass = BindClass::eNone;
		bool implicit = false;
		/* `@input` (true) or `@output` (false) */
		bool input = true;
		Condition when;
		String localName;
	};

	struct ParamDecl {
		String name;
		TypeRef type;
	};

	struct FunctionDecl {
		String name;
		Vector<ParamDecl> params;
		TypeRef ret;
		Opt<BodyRef> body;
		bool redefineAllowed = false;
		Condition when;
	};

	struct StructDecl {
		String name;
		Vector<FieldDesc> fields;
	};

	struct ToggleDecl {
		Toggle toggle;
	};

	using Decl = std::variant<BindingDecl, FunctionDecl, StructDecl, ToggleDecl>;

	struct Module {
		String name;
		EntryKind entry = EntryKind::eFragment;
		Vector<Toggle> toggles;
		Vector<TypeDesc> types;
		Vector<BindingDecl> bindings;
		Vector<FunctionDecl> functions;
		Vector<Expr> exprs;
		Vector<Stmt> stmts;
	};
}
