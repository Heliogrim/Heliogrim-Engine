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

	/* Arena handles; the arenas themselves are added with the AST */
	struct ExprRef {
		u32 index = ~0uL;
	};

	struct StmtRef {
		u32 index = ~0uL;
	};

	struct BodyRef {
		u32 index = ~0uL;
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
	};
}
