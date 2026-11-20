#pragma once

#include <spirv/unified1/spirv.hpp11>

/**/

#include <spirv/unified1/GLSL.std.450.h>

#include "Module.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

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
		eRequireMultiview = 1uL << 0uL,
		eRequireSparse = 1uL << 1uL,
		eRequireSparseClamp = 1uL << 2uL
	};

	/**
	 * A builtin variable (expression kind `eBuiltin`): its source name and type (scalar or vector).
	 */
	struct BuiltinVariable {
		StringView name;
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
		/* per-vertex input array (`input_position[i]`); `kind`/`n` describe one element */
		bool arrayed = false;
		/* fixed-size array of `kind`/`n` elements (`tess_level_outer`); 0 = not an array */
		u32 count = 0uL;
		/* stages that may assign it (when `writable`) */
		u8 writeStages = stageAll;
	};

	inline constexpr std::array<BuiltinVariable, 21> builtinVariables {
		// @formatter:off
		{
			{ "global_invocation_id", TypeKind::eVector, ScalarKind::eUInt, 3uL, stageCompute },
			{ "local_invocation_id", TypeKind::eVector, ScalarKind::eUInt, 3uL, stageCompute },
			{ "local_invocation_index", TypeKind::eScalar, ScalarKind::eUInt, 1uL, stageCompute },
			{ "workgroup_id", TypeKind::eVector, ScalarKind::eUInt, 3uL, stageCompute, false, eNoRequirement, true },
			{ "num_workgroups", TypeKind::eVector, ScalarKind::eUInt, 3uL, stageCompute, false, eNoRequirement, true },
			{ "vertex_index", TypeKind::eScalar, ScalarKind::eInt, 1uL, stageVertex },
			{ "instance_index", TypeKind::eScalar, ScalarKind::eInt, 1uL, stageVertex },
			{ "position", TypeKind::eVector, ScalarKind::eFloat, 4uL, stageVertex | stageTessEval | stageGeometry, true },
			{ "frag_depth", TypeKind::eScalar, ScalarKind::eFloat, 1uL, stageFragment, true },
			{ "frag_coord", TypeKind::eVector, ScalarKind::eFloat, 4uL, stageFragment },
			{ "front_facing", TypeKind::eScalar, ScalarKind::eBool, 1uL, stageFragment },
			{ "view_index", TypeKind::eScalar, ScalarKind::eInt, 1uL, stageGraphics, false, eRequireMultiview, true },
			{ "input_position", TypeKind::eVector, ScalarKind::eFloat, 4uL, stageGeometry | stageTessCtrl | stageTessEval, false, eNoRequirement, false, true },
			{ "output_position", TypeKind::eVector, ScalarKind::eFloat, 4uL, stageTessCtrl, true, eNoRequirement, false, true },
			{ "tess_level_outer", TypeKind::eScalar, ScalarKind::eFloat, 1uL, stageTessCtrl | stageTessEval, true, eNoRequirement, false, false, 4uL, stageTessCtrl },
			{ "tess_level_inner", TypeKind::eScalar, ScalarKind::eFloat, 1uL, stageTessCtrl | stageTessEval, true, eNoRequirement, false, false, 2uL, stageTessCtrl },
			{ "tess_coord", TypeKind::eVector, ScalarKind::eFloat, 3uL, stageTessEval },
			{ "patch_vertices", TypeKind::eScalar, ScalarKind::eInt, 1uL, stageTessCtrl | stageTessEval, false, eNoRequirement, true },
			{ "primitive_id", TypeKind::eScalar, ScalarKind::eInt, 1uL, stageGeometry },
			{ "invocation_id", TypeKind::eScalar, ScalarKind::eInt, 1uL, stageGeometry | stageTessCtrl },
			{ "layer", TypeKind::eScalar, ScalarKind::eInt, 1uL, stageGeometry, true }
		}
		// @formatter:on
	};

	[[nodiscard]] constexpr const BuiltinVariable* findBuiltinVariable(StringView name_) noexcept {
		const auto it = std::ranges::find(builtinVariables, name_, &BuiltinVariable::name);
		return it != builtinVariables.end() ? &*it : nullptr;
	}

	/**
	 * A builtin call. `signature` is `ret(p0,p1,...)` with concrete types (`f32`, `f32vec3`, `f32mat4`, `s32`, `bool`, ...)
	 * or families that expand together within one call: `genF` (f32, f32vec2/3/4), `genH` (f16), `genI` (s32), `genU` (u32),
	 * `genB` (bool) and `mat` (square f32 matrices). Texture calls use `tex` (any texture), `smp` (sampler), `coord` (f32 vector of
	 * the texture's coordinate width) and `size` (s32 vector of its size width); a leading `&` marks a `&mut` argument.
	 * The same name may appear several times (overloads, tried in order).
	 */
	struct BuiltinFunction {
		StringView name;
		StringView signature;
		u8 stages = stageAll;
		u32 requirement = eNoRequirement;
		/* implicit derivatives: only defined under uniform control flow */
		bool needsUniformControl = false;
		/* the divergence check is an error instead of a warning (lanes skipping the call deadlock the workgroup) */
		bool errorUnderDivergence = false;
	};

	inline constexpr BuiltinFunction builtinFunctions[] = {
		{ "sample", "f32vec4(tex,smp,coord)", stageAll, eNoRequirement, true },
		{ "sample_lod", "f32vec4(tex,smp,coord,f32)" },
		{ "texture_size", "size(tex,smp,s32)" },
		{ "sample_sparse", "s32(tex,smp,coord,&f32vec4)", stageAll, eRequireSparse, true },
		{
			"sample_sparse_clamp", "s32(tex,smp,coord,f32,&f32vec4)", stageAll,
			eRequireSparse | eRequireSparseClamp, true
		},
		{ "sparse_resident", "bool(s32)", stageAll, eRequireSparse },

		{ "abs", "genF(genF)" },
		{ "abs", "genI(genI)" },
		{ "sign", "genF(genF)" },
		{ "floor", "genF(genF)" },
		{ "ceil", "genF(genF)" },
		{ "fract", "genF(genF)" },
		{ "mod", "genF(genF,genF)" },
		{ "mod", "genF(genF,f32)" },
		{ "min", "genF(genF,genF)" },
		{ "min", "genF(genF,f32)" },
		{ "min", "genI(genI,genI)" },
		{ "min", "genU(genU,genU)" },
		{ "max", "genF(genF,genF)" },
		{ "max", "genF(genF,f32)" },
		{ "max", "genI(genI,genI)" },
		{ "max", "genU(genU,genU)" },
		{ "clamp", "genF(genF,genF,genF)" },
		{ "clamp", "genF(genF,f32,f32)" },
		{ "clamp", "genI(genI,genI,genI)" },
		{ "clamp", "genU(genU,genU,genU)" },
		{ "mix", "genF(genF,genF,genF)" },
		{ "mix", "genF(genF,genF,f32)" },
		{ "step", "genF(genF,genF)" },
		{ "step", "genF(f32,genF)" },
		{ "smoothstep", "genF(genF,genF,genF)" },
		{ "smoothstep", "genF(f32,f32,genF)" },

		{ "sqrt", "genF(genF)" },
		{ "inversesqrt", "genF(genF)" },
		{ "pow", "genF(genF,genF)" },
		{ "exp", "genF(genF)" },
		{ "log", "genF(genF)" },

		{ "sin", "genF(genF)" },
		{ "cos", "genF(genF)" },
		{ "tan", "genF(genF)" },
		{ "asin", "genF(genF)" },
		{ "acos", "genF(genF)" },
		{ "atan", "genF(genF)" },
		{ "atan", "genF(genF,genF)" },

		{ "length", "f32(genF)" },
		{ "distance", "f32(genF,genF)" },
		{ "dot", "f32(genF,genF)" },
		{ "cross", "f32vec3(f32vec3,f32vec3)" },
		{ "normalize", "genF(genF)" },
		{ "reflect", "genF(genF,genF)" },

		{ "transpose", "mat(mat)" },
		{ "inverse", "mat(mat)" },

		{ "dFdx", "genF(genF)", stageFragment, eNoRequirement, true },
		{ "dFdy", "genF(genF)", stageFragment, eNoRequirement, true },
		{ "fwidth", "genF(genF)", stageFragment, eNoRequirement, true },

		{ "barrier", "void()", stageCompute | stageTessCtrl, eNoRequirement, true, true },
		{ "memory_barrier_shared", "void()", stageCompute | stageTessCtrl, eNoRequirement, true, true },
		{ "emit_vertex", "void()", stageGeometry },
		{ "end_primitive", "void()", stageGeometry },
		{ "image_load", "ival(img,icoord)" },
		{ "image_store", "void(&img,icoord,ival)" },
		{ "image_size", "icoord(img)" },

		{ "atomic_add", "u32(&u32,u32)" },
		{ "atomic_add", "s32(&s32,s32)" },
		{ "atomic_min", "u32(&u32,u32)" },
		{ "atomic_min", "s32(&s32,s32)" },
		{ "atomic_max", "u32(&u32,u32)" },
		{ "atomic_max", "s32(&s32,s32)" },
		{ "atomic_and", "u32(&u32,u32)" },
		{ "atomic_and", "s32(&s32,s32)" },
		{ "atomic_or", "u32(&u32,u32)" },
		{ "atomic_or", "s32(&s32,s32)" },
		{ "atomic_xor", "u32(&u32,u32)" },
		{ "atomic_xor", "s32(&s32,s32)" },
		{ "atomic_exchange", "u32(&u32,u32)" },
		{ "atomic_exchange", "s32(&s32,s32)" },
		{ "atomic_comp_swap", "u32(&u32,u32,u32)" },
		{ "atomic_comp_swap", "s32(&s32,s32,s32)" }
	};

	[[nodiscard]] constexpr const BuiltinFunction* findBuiltinFunction(StringView name_) noexcept {
		const auto it = std::ranges::find(builtinFunctions, name_, &BuiltinFunction::name);
		return it != std::end(builtinFunctions) ? &*it : nullptr;
	}

	/* SPIR-V forms, indexed like `builtinVariables` / `builtinFunctions` */

	struct SpirvBuiltin {
		StringView name;
		spv::BuiltIn builtIn;
		spv::Capability capability = spv::Capability::Shader;
		StringView extension {};
	};

	inline constexpr std::array<SpirvBuiltin, 21> builtinVariableSpirv {
		{
			{ "global_invocation_id", spv::BuiltIn::GlobalInvocationId },
			{ "local_invocation_id", spv::BuiltIn::LocalInvocationId },
			{ "local_invocation_index", spv::BuiltIn::LocalInvocationIndex },
			{ "workgroup_id", spv::BuiltIn::WorkgroupId },
			{ "num_workgroups", spv::BuiltIn::NumWorkgroups },
			{ "vertex_index", spv::BuiltIn::VertexIndex },
			{ "instance_index", spv::BuiltIn::InstanceIndex },
			{ "position", spv::BuiltIn::Position },
			{ "frag_depth", spv::BuiltIn::FragDepth },
			{ "frag_coord", spv::BuiltIn::FragCoord },
			{ "front_facing", spv::BuiltIn::FrontFacing },
			{ "view_index", spv::BuiltIn::ViewIndex, spv::Capability::MultiView },
			{ "input_position", spv::BuiltIn::Position },
			{ "output_position", spv::BuiltIn::Position },
			{ "tess_level_outer", spv::BuiltIn::TessLevelOuter },
			{ "tess_level_inner", spv::BuiltIn::TessLevelInner },
			{ "tess_coord", spv::BuiltIn::TessCoord },
			{ "patch_vertices", spv::BuiltIn::PatchVertices },
			{ "primitive_id", spv::BuiltIn::PrimitiveId },
			{ "invocation_id", spv::BuiltIn::InvocationId },
			{ "layer", spv::BuiltIn::Layer }
		}
	};

	enum class SpirvFormKind : u8 {
		/* GLSL.std.450 extended instruction, `code` = GLSLstd450 */
		eExtInst,
		/* one core instruction, `code` = spv::Op */
		eCore,
		/* instruction sequence picked by the emitter, `code` = BuiltinLowering */
		eLower
	};

	enum class BuiltinLowering : u32 {
		eSample,
		eSampleLod,
		eTextureSize,
		eSampleSparse,
		eSampleSparseClamp,
		eDot,
		eBarrier,
		eMemoryBarrierShared
	};

	struct SpirvForm {
		SpirvFormKind kind;
		u32 code;
		spv::Capability capability = spv::Capability::Shader;
		StringView extension {};
	};

	struct BuiltinSpirv {
		StringView name;
		SpirvForm form;
	};

	namespace spirv_form {
		[[nodiscard]] constexpr SpirvForm ext(GLSLstd450 code_) noexcept {
			return { SpirvFormKind::eExtInst, static_cast<u32>(code_) };
		}

		[[nodiscard]] constexpr SpirvForm core(spv::Op op_, spv::Capability capability_ = spv::Capability::Shader) noexcept {
			return { SpirvFormKind::eCore, static_cast<u32>(op_), capability_ };
		}

		[[nodiscard]] constexpr SpirvForm lower(BuiltinLowering tag_, spv::Capability capability_ = spv::Capability::Shader) noexcept {
			return { SpirvFormKind::eLower, static_cast<u32>(tag_), capability_ };
		}
	}

	inline constexpr BuiltinSpirv builtinFunctionSpirv[] = {
		{ "sample", spirv_form::lower(BuiltinLowering::eSample) },
		{ "sample_lod", spirv_form::lower(BuiltinLowering::eSampleLod) },
		{ "texture_size", spirv_form::lower(BuiltinLowering::eTextureSize, spv::Capability::ImageQuery) },
		{ "sample_sparse", spirv_form::lower(BuiltinLowering::eSampleSparse, spv::Capability::SparseResidency) },
		{ "sample_sparse_clamp", spirv_form::lower(BuiltinLowering::eSampleSparseClamp, spv::Capability::MinLod) },
		{ "sparse_resident", spirv_form::core(spv::Op::OpImageSparseTexelsResident, spv::Capability::SparseResidency) },

		{ "abs", spirv_form::ext(GLSLstd450FAbs) },
		{ "abs", spirv_form::ext(GLSLstd450SAbs) },
		{ "sign", spirv_form::ext(GLSLstd450FSign) },
		{ "floor", spirv_form::ext(GLSLstd450Floor) },
		{ "ceil", spirv_form::ext(GLSLstd450Ceil) },
		{ "fract", spirv_form::ext(GLSLstd450Fract) },
		{ "mod", spirv_form::core(spv::Op::OpFMod) },
		{ "mod", spirv_form::core(spv::Op::OpFMod) },
		{ "min", spirv_form::ext(GLSLstd450FMin) },
		{ "min", spirv_form::ext(GLSLstd450FMin) },
		{ "min", spirv_form::ext(GLSLstd450SMin) },
		{ "min", spirv_form::ext(GLSLstd450UMin) },
		{ "max", spirv_form::ext(GLSLstd450FMax) },
		{ "max", spirv_form::ext(GLSLstd450FMax) },
		{ "max", spirv_form::ext(GLSLstd450SMax) },
		{ "max", spirv_form::ext(GLSLstd450UMax) },
		{ "clamp", spirv_form::ext(GLSLstd450FClamp) },
		{ "clamp", spirv_form::ext(GLSLstd450FClamp) },
		{ "clamp", spirv_form::ext(GLSLstd450SClamp) },
		{ "clamp", spirv_form::ext(GLSLstd450UClamp) },
		{ "mix", spirv_form::ext(GLSLstd450FMix) },
		{ "mix", spirv_form::ext(GLSLstd450FMix) },
		{ "step", spirv_form::ext(GLSLstd450Step) },
		{ "step", spirv_form::ext(GLSLstd450Step) },
		{ "smoothstep", spirv_form::ext(GLSLstd450SmoothStep) },
		{ "smoothstep", spirv_form::ext(GLSLstd450SmoothStep) },

		{ "sqrt", spirv_form::ext(GLSLstd450Sqrt) },
		{ "inversesqrt", spirv_form::ext(GLSLstd450InverseSqrt) },
		{ "pow", spirv_form::ext(GLSLstd450Pow) },
		{ "exp", spirv_form::ext(GLSLstd450Exp) },
		{ "log", spirv_form::ext(GLSLstd450Log) },

		{ "sin", spirv_form::ext(GLSLstd450Sin) },
		{ "cos", spirv_form::ext(GLSLstd450Cos) },
		{ "tan", spirv_form::ext(GLSLstd450Tan) },
		{ "asin", spirv_form::ext(GLSLstd450Asin) },
		{ "acos", spirv_form::ext(GLSLstd450Acos) },
		{ "atan", spirv_form::ext(GLSLstd450Atan) },
		{ "atan", spirv_form::ext(GLSLstd450Atan2) },

		{ "length", spirv_form::ext(GLSLstd450Length) },
		{ "distance", spirv_form::ext(GLSLstd450Distance) },
		{ "dot", spirv_form::lower(BuiltinLowering::eDot) },
		{ "cross", spirv_form::ext(GLSLstd450Cross) },
		{ "normalize", spirv_form::ext(GLSLstd450Normalize) },
		{ "reflect", spirv_form::ext(GLSLstd450Reflect) },

		{ "transpose", spirv_form::core(spv::Op::OpTranspose) },
		{ "inverse", spirv_form::ext(GLSLstd450MatrixInverse) },

		{ "dFdx", spirv_form::core(spv::Op::OpDPdx) },
		{ "dFdy", spirv_form::core(spv::Op::OpDPdy) },
		{ "fwidth", spirv_form::core(spv::Op::OpFwidth) },

		{ "barrier", spirv_form::lower(BuiltinLowering::eBarrier) },
		{ "memory_barrier_shared", spirv_form::lower(BuiltinLowering::eMemoryBarrierShared) },
		{ "emit_vertex", spirv_form::core(spv::Op::OpEmitVertex, spv::Capability::Geometry) },
		{ "end_primitive", spirv_form::core(spv::Op::OpEndPrimitive, spv::Capability::Geometry) },
		{ "image_load", spirv_form::core(spv::Op::OpImageRead) },
		{ "image_store", spirv_form::core(spv::Op::OpImageWrite) },
		{ "image_size", spirv_form::core(spv::Op::OpImageQuerySize, spv::Capability::ImageQuery) },

		{ "atomic_add", spirv_form::core(spv::Op::OpAtomicIAdd) },
		{ "atomic_add", spirv_form::core(spv::Op::OpAtomicIAdd) },
		{ "atomic_min", spirv_form::core(spv::Op::OpAtomicUMin) },
		{ "atomic_min", spirv_form::core(spv::Op::OpAtomicSMin) },
		{ "atomic_max", spirv_form::core(spv::Op::OpAtomicUMax) },
		{ "atomic_max", spirv_form::core(spv::Op::OpAtomicSMax) },
		{ "atomic_and", spirv_form::core(spv::Op::OpAtomicAnd) },
		{ "atomic_and", spirv_form::core(spv::Op::OpAtomicAnd) },
		{ "atomic_or", spirv_form::core(spv::Op::OpAtomicOr) },
		{ "atomic_or", spirv_form::core(spv::Op::OpAtomicOr) },
		{ "atomic_xor", spirv_form::core(spv::Op::OpAtomicXor) },
		{ "atomic_xor", spirv_form::core(spv::Op::OpAtomicXor) },
		{ "atomic_exchange", spirv_form::core(spv::Op::OpAtomicExchange) },
		{ "atomic_exchange", spirv_form::core(spv::Op::OpAtomicExchange) },
		{ "atomic_comp_swap", spirv_form::core(spv::Op::OpAtomicCompareExchange) },
		{ "atomic_comp_swap", spirv_form::core(spv::Op::OpAtomicCompareExchange) }
	};

	static_assert(std::size(builtinFunctionSpirv) == std::size(builtinFunctions));
	static_assert(
		[] {
			for (auto i = 0uz; i < std::size(builtinFunctions); ++i) {
				if (builtinFunctionSpirv[i].name != builtinFunctions[i].name) {
					return false;
				}
				// shape: the form has to fit the overload's parameter list, not just its name
				const auto sig = StringView { builtinFunctions[i].signature };
				const auto form = builtinFunctionSpirv[i].form;
				const auto byRef = sig.contains("(&") && not sig.contains("img");
				const auto atomic = form.kind == SpirvFormKind::eCore
					&& form.code >= static_cast<u32>(spv::Op::OpAtomicLoad) && form.code <= static_cast<u32>(spv::Op::OpAtomicXor);
				if (atomic != byRef) {
					return false;
				}
				if (form.kind == SpirvFormKind::eExtInst
					&& (sig.starts_with("void(") || sig.contains("&") || sig.contains("tex") || sig.contains("img"))) {
					return false;
				}
				if (form.kind == SpirvFormKind::eLower) {
					using enum BuiltinLowering;
					const auto lowering = static_cast<BuiltinLowering>(form.code);
					const auto sampled = lowering == eSample || lowering == eSampleLod || lowering == eSampleSparse
						|| lowering == eSampleSparseClamp || lowering == eTextureSize;
					if (sampled != sig.contains("(tex,smp,")) {
						return false;
					}
				}
			}
			for (auto i = 0uz; i < builtinVariables.size(); ++i) {
				if (builtinVariableSpirv[i].name != builtinVariables[i].name) {
					return false;
				}
			}
			return true;
		}(),
		"SPIR-V builtin tables must line up with the builtin tables"
	);

	/* parsed signatures */

	enum class BuiltinFamily : u8 {
		eConcrete,
		eGenF,
		eGenH,
		eGenI,
		eGenU,
		eGenB,
		eMat,
		eTexture,
		eSampler,
		eCoord,
		eSize,
		/* storage image, its s32 coordinate / size, its texel (format dependent) and no value */
		eImage,
		eImageCoord,
		eImageValue,
		eVoid
	};

	struct BuiltinSigType {
		BuiltinFamily family = BuiltinFamily::eConcrete;
		/* `&mut` argument */
		bool mut = false;
		TypeKind kind = TypeKind::eScalar;
		ScalarKind scalar = ScalarKind::eFloat;
		u32 bits = 32uL;
		u32 n = 1uL;
		u32 m = 1uL;
	};

	struct BuiltinSignature {
		BuiltinSigType ret;
		std::array<BuiltinSigType, 5> params;
		u32 count = 0uL;
		bool valid = false;
	};

	[[nodiscard]] inline BuiltinSigType parseBuiltinSigType(StringView text_) {
		using enum BuiltinFamily;
		BuiltinSigType t {};
		if (text_.starts_with('&')) {
			t.mut = true;
			text_.remove_prefix(1uz);
		}
		if (text_ == "tex" || text_ == "smp" || text_ == "coord" || text_ == "size") {
			t.family = text_ == "tex" ? eTexture : text_ == "smp" ? eSampler : text_ == "coord" ? eCoord : eSize;
			t.scalar = t.family == eSize ? ScalarKind::eInt : ScalarKind::eFloat;
		} else if (text_ == "img" || text_ == "icoord" || text_ == "ival" || text_ == "void") {
			t.family = text_ == "img" ? eImage : text_ == "icoord" ? eImageCoord : text_ == "ival" ? eImageValue : eVoid;
			t.scalar = t.family == eImageCoord ? ScalarKind::eInt : ScalarKind::eFloat;
		} else if (text_ == "genF" || text_ == "genH") {
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

	/* whether argument `at_` of builtin `name_` is a `&mut` out parameter (e.g. the texel of `sample_sparse`) */
	[[nodiscard]] inline bool builtinWritesArg(StringView name_, size_t at_) {
		const auto* fn = findBuiltinFunction(name_);
		if (fn == nullptr) {
			return false;
		}
		const auto& sig = parsedBuiltinSignatures()[static_cast<size_t>(fn - std::begin(builtinFunctions))];
		return at_ < sig.count && sig.params[at_].mut;
	}

	/* the concrete type of a signature slot for family width `width_` (1-4, matrices 2-4); none when the family has no such width */
	[[nodiscard]] inline Opt<TypeDesc> resolveBuiltinSigType(
		const BuiltinSigType& t_,
		u32 width_,
		TextureDim dim_ = TextureDim::e2D,
		StringView format_ = {}
	) {
		TypeDesc desc {};
		desc.scalar = t_.scalar;
		desc.bits = t_.bits;
		switch (t_.family) {
			case BuiltinFamily::eImage: {
				desc.kind = TypeKind::eImage;
				desc.dim = dim_;
				desc.name = String(format_);
				return desc;
			}
			case BuiltinFamily::eImageCoord: {
				const auto n = dim_ == TextureDim::e1D ? 1uL : dim_ == TextureDim::e2D ? 2uL : 3uL;
				desc.kind = n == 1uL ? TypeKind::eScalar : TypeKind::eVector;
				desc.n = n;
				return desc;
			}
			case BuiltinFamily::eImageValue: {
				desc.kind = TypeKind::eVector;
				desc.n = 4uL;
				desc.scalar = format_.ends_with("ui") ?
					ScalarKind::eUInt :
					(format_.ends_with("i") ? ScalarKind::eInt : ScalarKind::eFloat);
				return desc;
			}
			case BuiltinFamily::eVoid: return desc;
			case BuiltinFamily::eTexture: {
				desc.kind = TypeKind::eTexture;
				desc.dim = dim_;
				return desc;
			}
			case BuiltinFamily::eSampler: {
				desc.kind = TypeKind::eSampler;
				return desc;
			}
			case BuiltinFamily::eCoord:
			case BuiltinFamily::eSize: {
				const auto coord = t_.family == BuiltinFamily::eCoord;
				const auto n = dim_ == TextureDim::e1D ?
					1uL :
					dim_ == TextureDim::e2D ?
					2uL :
					dim_ == TextureDim::eCube ?
					(coord ? 3uL : 2uL) :
					3uL;
				desc.kind = n == 1uL ? TypeKind::eScalar : TypeKind::eVector;
				desc.n = n;
				return desc;
			}
			default: break;
		}
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
