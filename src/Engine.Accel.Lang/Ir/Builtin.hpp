#pragma once

#include <algorithm>
#include <array>
#include <Engine.Common/String.hpp>
#include <Engine.Common/Types.hpp>

#include "Module.hpp"

namespace hg::engine::accel::lang::ir {
	/**
	 * A builtin variable (expression kind `eBuiltin`): its source name, GLSL name and type (scalar or vector).
	 */
	struct BuiltinVariable {
		StringView name;
		StringView glsl;
		TypeKind kind;
		ScalarKind scalar;
		u32 n;
	};

	inline constexpr std::array<BuiltinVariable, 4> builtinVariables {
		{
			{ "vertex_index", "gl_VertexIndex", TypeKind::eScalar, ScalarKind::eInt, 1uL },
			{ "instance_index", "gl_InstanceIndex", TypeKind::eScalar, ScalarKind::eInt, 1uL },
			{ "position", "gl_Position", TypeKind::eVector, ScalarKind::eFloat, 4uL },
			{ "frag_depth", "gl_FragDepth", TypeKind::eScalar, ScalarKind::eFloat, 1uL }
		}
	};

	[[nodiscard]] constexpr const BuiltinVariable* findBuiltinVariable(StringView name_) noexcept {
		const auto it = std::ranges::find(builtinVariables, name_, &BuiltinVariable::name);
		return it != builtinVariables.end() ? &*it : nullptr;
	}

	/* builtin calls; `sample(texture, sampler, uv)` emits `texture(sampler2D(texture, sampler), uv)` (dimension of the texture) */
	inline constexpr std::array<StringView, 1> builtinFunctions { "sample" };
}
