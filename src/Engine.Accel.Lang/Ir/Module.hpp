#pragma once

#include <array>
#include <variant>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/Optional.hpp>
#include <Engine.Common/String.hpp>
#include <Engine.Common/Types.hpp>

#include "Condition.hpp"
#include "Diagnostic.hpp"

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
		ePushConstant,
		eStorageImage,
		/* module-scope `workgroup` variable; not part of any interface */
		eWorkgroup
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

	struct ConstantRef {
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
		eConstruct,
		/* builtin variable (`position` -> gl_Position) or builtin call (`sample(...)`) by `name`; never a binding use */
		eBuiltin,
		/* operands = [cond, then, else] */
		eTernary
	};

	/**
	 * Flat expression node; only the members relevant to `kind` are meaningful.
	 */
	struct Expr {
		ExprKind kind = ExprKind::eLiteral;

		/* literal: source text (`true`/`false` included); nameref/call/builtin: referenced name; field: member name
		 * (swizzles too); unary/binary: operator text */
		String name;

		/* nameref: resolved binding; call: resolved function; ~0 = unresolved (then matched by name).
		 * Typed so arena merging can remap them like the other refs. */
		BindingRef binding;
		FunctionRef function;
		/* nameref: resolved module or spec constant */
		ConstantRef constant;

		/* nameref resolved to a local or parameter: not a binding use, `binding` stays absent */
		bool local = false;

		/* every expression after typecheck (construct: set at parse) */
		TypeRef type;

		/* call/construct/builtin call: arguments; field: [base]; index: [base, index]; unary: [operand]; binary: [lhs, rhs] */
		Vector<ExprRef> operands;

		/* head token; diagnostics only, not part of the content hash */
		SourceLoc loc;
	};

	enum class StmtKind : u8 {
		eDeclareLocal,
		eAssign,
		eExpr,
		eReturn,
		eBlock,
		eWhen,
		eIf,
		/* local reference alias: name, lhs = place, mutable_ = `&mut` */
		eDeclareRef,
		/* children = [init, body, step] (init/step an empty block when absent), lhs = condition (absent = missing) */
		eFor,
		/* lhs = condition, children = [body] */
		eWhile,
		eBreak,
		eContinue,
		eDiscard
	};

	/**
	 * Flat statement node; only the members relevant to `kind` are meaningful.
	 */
	struct Stmt {
		StmtKind kind = StmtKind::eExpr;

		/* declare-local; the inferred type of a `let` after typecheck */
		String name;
		TypeRef type;

		/* assign: `name` is the operator (`+=`; empty = plain `=`). declare-local: init (optional), assign: lhs, expr/return: value (optional for return), if: condition */
		ExprRef lhs;
		/* assign: rhs */
		ExprRef rhs;

		/* declare-local: `let` / `T x` (true) or `const` (false); declare-ref: `&mut` */
		bool mutable_ = true;

		/* block: children; when: [then, else?] (the parser emits [then]); if: [then, else?] */
		Vector<StmtRef> children;
		Condition when;

		/* head token; diagnostics only, not part of the content hash */
		SourceLoc loc;
	};

	enum class TypeKind : u8 {
		eScalar,
		eVector,
		eMatrix,
		eArray,
		eStruct,
		eTexture,
		eSampler,
		/* storage image: `dim` plus the format in `TypeDesc::name` */
		eImage
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
		eCube,
		e2DArray
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

	enum class Access : u8 {
		eRead,
		eWrite,
		eReadWrite
	};

	enum class ParamKind : u8 {
		eValue,
		eRef,
		eRefMut
	};

	enum class Packing : u8 {
		eDefault,
		eStd140,
		eStd430
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
		Access access = Access::eRead;
		Packing packing = Packing::eDefault;
		/* per-vertex forward input of geometry/tessellation (`T[] name`); `type` is the element type */
		bool arrayed = false;
		/* per-patch forward output of tess_ctrl / input of tess_eval (`patch out` / `patch in`) */
		bool patch = false;
	};

	/* module-scope `const`; a `specId` makes it a specialization constant (the layout pass assigns its constant_id) */
	struct ConstantDecl {
		String name;
		TypeRef type;
		ExprRef value;
		Opt<String> specId;
		Condition when;

		/* name token; diagnostics only, not part of the content hash */
		SourceLoc loc;
	};

	struct ParamDecl {
		String name;
		TypeRef type;
		ParamKind kind = ParamKind::eValue;
	};

	struct FunctionDecl {
		String name;
		Vector<ParamDecl> params;
		TypeRef ret;
		Opt<BodyRef> body;
		bool redefineAllowed = false;
		Condition when;

		/* name token; diagnostics only, not part of the content hash */
		SourceLoc loc;
	};

	struct StructDecl {
		String name;
		Vector<FieldDesc> fields;
	};

	struct ToggleDecl {
		Toggle toggle;
	};

	using Decl = std::variant<BindingDecl, FunctionDecl, StructDecl, ToggleDecl>;

	/* execution attributes of `fn main` */
	enum class Primitive : u8 {
		ePoints,
		eLines,
		eTriangles,
		eLinesAdjacency,
		eTrianglesAdjacency,
		eLineStrip,
		eTriangleStrip
	};

	enum class TessDomain : u8 {
		eTriangles,
		eQuads,
		eIsolines
	};

	enum class TessSpacing : u8 {
		eEqual,
		eFractionalEven,
		eFractionalOdd
	};

	struct EntryAttributes {
		std::array<u32, 3> localSize { 1uL, 1uL, 1uL };
		bool earlyFragmentTests = false;
		/* geometry */
		Primitive geometryInput = Primitive::eTriangles;
		Primitive geometryOutput = Primitive::eTriangleStrip;
		u32 maxVertices = 0uL;
		u32 invocations = 1uL;
		/* tess_ctrl */
		u32 outputVertices = 0uL;
		/* tess_eval */
		TessDomain tessDomain = TessDomain::eTriangles;
		TessSpacing tessSpacing = TessSpacing::eEqual;
		bool clockwise = false;

		[[nodiscard]] bool operator==(const EntryAttributes&) const noexcept = default;
	};

	struct Module {
		String name;
		EntryKind entry = EntryKind::eFragment;
		EntryAttributes attributes;
		Vector<Toggle> toggles;
		Vector<TypeDesc> types;
		Vector<BindingDecl> bindings;
		Vector<ConstantDecl> constants;
		Vector<FunctionDecl> functions;
		Vector<Expr> exprs;
		Vector<Stmt> stmts;

		/* non-fatal diagnostics from parse/typecheck (e.g. implicit-derivative sampling under divergence); not hashed; merge and compact drop them, so consumers read them from the parsed input modules */
		Diagnostics warnings;
	};
}
