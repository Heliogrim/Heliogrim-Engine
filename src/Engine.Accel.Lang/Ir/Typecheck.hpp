#pragma once

#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/String.hpp>
#include <Engine.Common/Types.hpp>
#include <Engine.Common/Wrapper.hpp>

#include "Diagnostic.hpp"
#include "Module.hpp"

namespace hg::engine::accel::lang::ir {
	/**
	 * Type name in `.asl` spelling (`f32`, `f32vec3`, `f32mat4x3`, `T[4]`, a struct name, `texture2d`, `sampler`);
	 * `<none>` for an invalid ref.
	 */
	[[nodiscard]] String describeType(ref<const Vector<TypeDesc>> types_, TypeRef type_, u32 depth_ = 0uL);

	/**
	 * Function identity: name and parameter types, e.g. `saturate(f32vec3)`. The return type is not part of it.
	 */
	[[nodiscard]] String signatureOf(ref<const Module> module_, ref<const FunctionDecl> fn_);

	/**
	 * Types every expression (`Expr::type`), every `let` without declared type (`Stmt::type`) and resolves each
	 * `eCall` to a `FunctionRef` by exact parameter types; a literal adapts to the parameter type otherwise
	 * (an integer literal to any float or integer type, a float literal to float types only).
	 * Never asserts; diagnostics carry no source location (the arenas hold none).
	 */
	[[nodiscard]] Diagnostics typecheck(ref<Module> module_);
}
