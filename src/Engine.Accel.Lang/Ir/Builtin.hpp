#pragma once

#include <algorithm>
#include <array>
#include <iterator>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/String.hpp>
#include <Engine.Common/Types.hpp>

#include "Module.hpp"

namespace hg::engine::accel::lang::ir {
	/* `stages` masks: one bit per `EntryKind` */
	[[nodiscard]] constexpr u8 stageBit(const EntryKind kind_) noexcept {
		return static_cast<u8>(1uL << static_cast<u8>(kind_));
	}

	inline constexpr u8 stageVertex = 1u << 0u;
	inline constexpr u8 stageTessCtrl = 1u << 1u;
	inline constexpr u8 stageTessEval = 1u << 2u;
	inline constexpr u8 stageGeometry = 1u << 3u;
	inline constexpr u8 stageFragment = 1u << 4u;
	inline constexpr u8 stageCompute = 1u << 5u;
	inline constexpr u8 stageGraphics = stageVertex | stageTessCtrl | stageTessEval | stageGeometry | stageFragment;
	inline constexpr u8 stageAll = stageGraphics | stageCompute;

	[[nodiscard]] constexpr StringView stageName(const EntryKind kind_) noexcept {
		constexpr std::array<StringView, 6> names {
			"vertex", "tessellation control", "tessellation evaluation", "geometry", "fragment", "compute"
		};
		return names[static_cast<u8>(kind_)];
	}

	/* extension requirements of a builtin, mapped to `#extension` lines by the emitter */
	enum BuiltinRequirement : u32 {
		eNoRequirement = 0uL,
		eRequireMultiview = 1uL << 0uL
	};

	/**
	 * A builtin variable (expression kind `eBuiltin`): its source name, GLSL name and type (scalar or vector).
	 */
	struct BuiltinVariable {
		StringView name;
		StringView glsl;
		TypeKind kind;
		ScalarKind scalar;
		u32 n;
		/* stages the variable exists in (bit per `EntryKind`) */
		u8 stages = stageAll;
		/* may be assigned (in `stages`) */
		bool writable = false;
		u32 requirement = eNoRequirement;
		/* same value across all invocations of a workgroup/draw (uniform control analysis) */
		bool uniform = false;
	};

	inline constexpr std::array<BuiltinVariable, 7> builtinVariables {
		{
			{ "vertex_index", "gl_VertexIndex", TypeKind::eScalar, ScalarKind::eInt, 1uL, stageVertex },
			{ "instance_index", "gl_InstanceIndex", TypeKind::eScalar, ScalarKind::eInt, 1uL, stageVertex },
			{ "position", "gl_Position", TypeKind::eVector, ScalarKind::eFloat, 4uL, stageVertex | stageTessEval | stageGeometry, true },
			{ "frag_depth", "gl_FragDepth", TypeKind::eScalar, ScalarKind::eFloat, 1uL, stageFragment, true },
			{ "frag_coord", "gl_FragCoord", TypeKind::eVector, ScalarKind::eFloat, 4uL, stageFragment },
			{ "front_facing", "gl_FrontFacing", TypeKind::eScalar, ScalarKind::eBool, 1uL, stageFragment },
			{ "view_index", "gl_ViewIndex", TypeKind::eScalar, ScalarKind::eInt, 1uL, stageGraphics, false, eRequireMultiview, true }
		}
	};

	[[nodiscard]] constexpr const BuiltinVariable* findBuiltinVariable(StringView name_) noexcept {
		const auto it = std::ranges::find(builtinVariables, name_, &BuiltinVariable::name);
		return it != builtinVariables.end() ? &*it : nullptr;
	}

	/**
	 * A builtin call. `signature` is `ret(p0,p1,...)` with concrete types (`f32`, `f32vec3`, `f32mat4`, `s32`, `bool`, ...)
	 * or families that expand together within one call: `genF` (f32, f32vec2/3/4), `genH` (f16), `genI` (s32), `genU` (u32),
	 * `genB` (bool) and `mat` (square f32 matrices). An empty signature is handled by the typechecker (`sample`).
	 * The same name may appear several times (overloads, tried in order).
	 */
	struct BuiltinFunction {
		StringView name;
		StringView glsl;
		StringView signature;
		u8 stages = stageAll;
		u32 requirement = eNoRequirement;
		/* implicit derivatives: only defined under uniform control flow */
		bool needsUniformControl = false;
	};

	inline constexpr BuiltinFunction builtinFunctions[] = {
		{ "sample", "texture", "", stageAll, eNoRequirement, false },

		{ "abs", "abs", "genF(genF)" },
		{ "abs", "abs", "genI(genI)" },
		{ "sign", "sign", "genF(genF)" },
		{ "floor", "floor", "genF(genF)" },
		{ "ceil", "ceil", "genF(genF)" },
		{ "fract", "fract", "genF(genF)" },
		{ "mod", "mod", "genF(genF,genF)" },
		{ "mod", "mod", "genF(genF,f32)" },
		{ "min", "min", "genF(genF,genF)" },
		{ "min", "min", "genF(genF,f32)" },
		{ "min", "min", "genI(genI,genI)" },
		{ "min", "min", "genU(genU,genU)" },
		{ "max", "max", "genF(genF,genF)" },
		{ "max", "max", "genF(genF,f32)" },
		{ "max", "max", "genI(genI,genI)" },
		{ "max", "max", "genU(genU,genU)" },
		{ "clamp", "clamp", "genF(genF,genF,genF)" },
		{ "clamp", "clamp", "genF(genF,f32,f32)" },
		{ "clamp", "clamp", "genI(genI,genI,genI)" },
		{ "clamp", "clamp", "genU(genU,genU,genU)" },
		{ "mix", "mix", "genF(genF,genF,genF)" },
		{ "mix", "mix", "genF(genF,genF,f32)" },
		{ "step", "step", "genF(genF,genF)" },
		{ "step", "step", "genF(f32,genF)" },
		{ "smoothstep", "smoothstep", "genF(genF,genF,genF)" },
		{ "smoothstep", "smoothstep", "genF(f32,f32,genF)" },

		{ "sqrt", "sqrt", "genF(genF)" },
		{ "inversesqrt", "inversesqrt", "genF(genF)" },
		{ "pow", "pow", "genF(genF,genF)" },
		{ "exp", "exp", "genF(genF)" },
		{ "log", "log", "genF(genF)" },

		{ "sin", "sin", "genF(genF)" },
		{ "cos", "cos", "genF(genF)" },
		{ "tan", "tan", "genF(genF)" },
		{ "asin", "asin", "genF(genF)" },
		{ "acos", "acos", "genF(genF)" },
		{ "atan", "atan", "genF(genF)" },
		{ "atan", "atan", "genF(genF,genF)" },

		{ "length", "length", "f32(genF)" },
		{ "distance", "distance", "f32(genF,genF)" },
		{ "dot", "dot", "f32(genF,genF)" },
		{ "cross", "cross", "f32vec3(f32vec3,f32vec3)" },
		{ "normalize", "normalize", "genF(genF)" },
		{ "reflect", "reflect", "genF(genF,genF)" },

		{ "transpose", "transpose", "mat(mat)" },
		{ "inverse", "inverse", "mat(mat)" },

		{ "dFdx", "dFdx", "genF(genF)", stageFragment, eNoRequirement, true },
		{ "dFdy", "dFdy", "genF(genF)", stageFragment, eNoRequirement, true },
		{ "fwidth", "fwidth", "genF(genF)", stageFragment, eNoRequirement, true }
	};

	[[nodiscard]] constexpr const BuiltinFunction* findBuiltinFunction(StringView name_) noexcept {
		const auto it = std::ranges::find(builtinFunctions, name_, &BuiltinFunction::name);
		return it != std::end(builtinFunctions) ? &*it : nullptr;
	}

	/* parsed signatures */

	enum class BuiltinFamily : u8 {
		eConcrete,
		eGenF,
		eGenH,
		eGenI,
		eGenU,
		eGenB,
		eMat
	};

	struct BuiltinSigType {
		BuiltinFamily family = BuiltinFamily::eConcrete;
		TypeKind kind = TypeKind::eScalar;
		ScalarKind scalar = ScalarKind::eFloat;
		u32 bits = 32uL;
		u32 n = 1uL;
		u32 m = 1uL;
	};

	struct BuiltinSignature {
		BuiltinSigType ret;
		std::array<BuiltinSigType, 4> params;
		u32 count = 0uL;
		bool valid = false;
	};

	[[nodiscard]] inline BuiltinSigType parseBuiltinSigType(StringView text_) {
		using enum BuiltinFamily;
		BuiltinSigType t {};
		if (text_ == "genF" || text_ == "genH") {
			t.family = text_ == "genF" ? eGenF : eGenH;
			t.bits = text_ == "genF" ? 32uL : 16uL;
		} else if (text_ == "genI" || text_ == "genU" || text_ == "genB") {
			t.family = text_ == "genI" ? eGenI : (text_ == "genU" ? eGenU : eGenB);
			t.scalar = text_ == "genI" ? ScalarKind::eInt : (text_ == "genU" ? ScalarKind::eUInt : ScalarKind::eBool);
		} else if (text_ == "mat") {
			t.family = eMat;
			t.kind = TypeKind::eMatrix;
		} else if (text_ == "bool") {
			t.scalar = ScalarKind::eBool;
		} else if (not text_.empty()) {
			t.scalar = text_[0] == 'f' ? ScalarKind::eFloat : (text_[0] == 's' ? ScalarKind::eInt : ScalarKind::eUInt);
			auto i = 1uz;
			t.bits = 0uL;
			for (; i < text_.size() && text_[i] >= '0' && text_[i] <= '9'; ++i) {
				t.bits = t.bits * 10uL + static_cast<u32>(text_[i] - '0');
			}
			const auto rest = text_.substr(i);
			if (rest.size() == 4uz && rest.starts_with("vec")) {
				t.kind = TypeKind::eVector;
				t.n = static_cast<u32>(rest[3] - '0');
			} else if (rest.size() == 4uz && rest.starts_with("mat")) {
				t.kind = TypeKind::eMatrix;
				t.n = t.m = static_cast<u32>(rest[3] - '0');
			}
		}
		return t;
	}

	[[nodiscard]] inline BuiltinSignature parseBuiltinSignature(StringView text_) {
		BuiltinSignature sig {};
		const auto open = text_.find('(');
		if (open == StringView::npos || not text_.ends_with(')')) {
			return sig;
		}
		sig.ret = parseBuiltinSigType(text_.substr(0uz, open));
		auto rest = text_.substr(open + 1uz, text_.size() - open - 2uz);
		while (not rest.empty() && sig.count < sig.params.size()) {
			const auto comma = rest.find(',');
			sig.params[sig.count++] = parseBuiltinSigType(rest.substr(0uz, comma));
			rest = comma == StringView::npos ? StringView {} : rest.substr(comma + 1uz);
		}
		sig.valid = rest.empty();
		return sig;
	}

	/* one parsed signature per `builtinFunctions` entry, built once */
	[[nodiscard]] inline const Vector<BuiltinSignature>& parsedBuiltinSignatures() {
		static const auto parsed = [] {
			Vector<BuiltinSignature> all {};
			for (const auto& fn : builtinFunctions) {
				all.emplace_back(parseBuiltinSignature(fn.signature));
			}
			return all;
		}();
		return parsed;
	}

	/* the concrete type of a signature slot for family width `width_` (1-4, matrices 2-4); none when the family has no such width */
	[[nodiscard]] inline Opt<TypeDesc> resolveBuiltinSigType(const BuiltinSigType& t_, u32 width_) {
		TypeDesc desc {};
		desc.scalar = t_.scalar;
		desc.bits = t_.bits;
		if (t_.family == BuiltinFamily::eConcrete) {
			desc.kind = t_.kind;
			desc.n = t_.n;
			desc.m = t_.m;
			return desc;
		}
		if (t_.family == BuiltinFamily::eMat) {
			if (width_ < 3uL) {
				return None;
			}
			desc.kind = TypeKind::eMatrix;
			desc.n = desc.m = width_;
			return desc;
		}
		desc.kind = width_ == 1uL ? TypeKind::eScalar : TypeKind::eVector;
		desc.n = width_;
		return desc;
	}
}
