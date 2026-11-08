#include "pch.h"

#include "IrSpirvGate.hpp"

using namespace hg::engine::accel::lang::ir;

namespace AccelCompile {
	TEST(IrSpirvGateTest, RejectsBrokenGlsl) {
		EXPECT_FALSE(glslCompiles("#version 460\nvoid main(){ gl_Position = vec4(0); }", EntryKind::eFragment));
	}

	TEST(IrSpirvGateTest, VertexSample) {
		EXPECT_TRUE(
			compilesToSpirv(
				"@input (mode = bind, id = \"view\", class = uniform) block camera { f32mat4 mvp; f32mat4 proj; f32mat4 view; };\n"
				"@input (mode = bind, id = \"cascades\", class = storage) block cascades { f32mat4[4] viewProj; f32vec4 depth; };\n"
				"@input (mode = bind, id = \"model\", class = storage) block model { f32mat4[] model; };\n"
				"@input (mode = bind, id = \"block\", class = push) block push { f32vec2 translate; u32 cascade; };\n"
				"@input (mode = forward, id = \"vertex-position\") f32vec3 local_position;\n"
				"@input (mode = forward, id = \"vertex-normal\") f32vec3 local_normal;\n"
				"@output (mode = forward, id = \"world-normal\") f32vec4 world_normal;\n"
				"@output (mode = forward, id = \"local-uv\") f32vec2 out_uv;\n"
				"const UVS: f32vec2[3] = f32vec2[3](f32vec2(0.0), f32vec2(1.0, 0.0), f32vec2(0.0, 1.0));\n"
				"fn main() {\n"
				"	let m = model.model[instance_index];\n"
				"	let shift = f32vec2(0.0);\n"
				"	if (vertex_index == 0) { shift = push.translate; }\n"
				"	let pos = m * f32vec4(local_position + f32vec3(shift, 0.0), 1.0);\n"
				"	position = cascades.viewProj[push.cascade] * camera.mvp * pos;\n"
				"	world_normal = f32vec4(normalize(transpose(inverse(f32mat3(m))) * local_normal), 0.0);\n"
				"	out_uv = UVS[vertex_index];\n"
				"}\n",
				EntryKind::eVertex
			)
		);
	}

	TEST(IrSpirvGateTest, DepthOnlyFragment) {
		EXPECT_TRUE(compilesToSpirv("@output (mode = forward, id = \"depth\") implicit f32 depth;\n", EntryKind::eFragment));
		EXPECT_TRUE(
			compilesToSpirv(
				"@output (mode = forward, id = \"depth\") implicit f32 depth;\n@fragment (early_fragment_tests = true)\nfn main() {}\n",
				EntryKind::eFragment
			)
		);
	}

	TEST(IrSpirvGateTest, LightLoopSample) {
		EXPECT_TRUE(
			compilesToSpirv(
				"#include <shared/math.asl>\n"
				"struct Light { f32vec4 luminance; f32vec4 position; f32 limit; };\n"
				"struct Shadow { f32mat4[4] viewProj; f32vec4 depth; };\n"
				"@input (mode = bind, id = \"view\", class = uniform) block camera { f32mat4 view; };\n"
				"@input (mode = bind, id = \"light-info\", class = uniform) block info { u32 count; };\n"
				"@input (mode = bind, id = \"lights\", class = storage, packing = std140) block sl { Light[] lights; };\n"
				"@input (mode = bind, id = \"shadows\", class = storage, packing = std140) block shadows { Shadow[] shadows; };\n"
				"@input (mode = bind, id = \"shadow-dir\", class = texture) texture2darray shadow_dir;\n"
				"@input (mode = bind, id = \"shadow-dir-sampler\", class = sampler) sampler shadow_smp;\n"
				"@input (mode = forward, id = \"world-position\") f32vec4 world_position;\n"
				"@output (mode = forward, id = \"color\") f32vec4 color;\n"
				"@output (mode = forward, id = \"depth\") implicit f32 depth;\n"
				"fn falloff(l: Light, p: f32vec3) -> f32vec3 {\n"
				"	return l.luminance.rgb * saturate(1.0 - length(l.position.xyz - p) / l.limit);\n"
				"}\n"
				"fn main() {\n"
				"	let eye = inverse(camera.view)[3].xyz;\n"
				"	let tndc = f32mat4(0.5, 0.0, 0.0, 0.0, 0.0, 0.5, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.5, 0.5, 0.0, 1.0);\n"
				"	let lo = f32vec3(0.0);\n"
				"	for (let i = 0u; i < info.count; ++i) {\n"
				"		let l = &sl.lights[i];\n"
				"		if (l.limit > 0.0) { lo += falloff(l, world_position.xyz); } else { lo += l.luminance.rgb; }\n"
				"	}\n"
				"	let s = tndc * shadows.shadows[0].viewProj[1] * world_position;\n"
				"	let d = sample(shadow_dir, shadow_smp, f32vec3(s.xy / s.w, 1.0)).r;\n"
				"	let shade = s.z > d ? 0.07 : 1.0;\n"
				"	color = f32vec4(saturate(lo * shade) / max(length(eye - world_position.xyz), 1.0), PI);\n"
				"}\n",
				EntryKind::eFragment,
				{},
				{ { "shared/math.asl",
					"const PI: f32 = 3.14159265;\n"
					"fn saturate(x: f32) -> f32 { return clamp(x, 0.0, 1.0); }\n"
					"fn saturate(x: f32vec3) -> f32vec3 { return clamp(x, 0.0, 1.0); }\n"
					"fn twice(x: f16) -> f16 { return x * f16(2.0); }\n" } }
			)
		);
	}

	namespace {
		/* mesh-type contract split over includes: prototypes, then one @when body set per MeshType */
		constexpr hg::StringView meshContract =
			"toggle MeshType : { Static, Skeletal } = Static;\n"
			"fn GetWorldPosition() -> f32vec4;\n"
			"fn VertexOutputPosition() -> f32vec4;\n";
		constexpr hg::StringView meshStatic =
			"#include <spec/defined.asl>\n"
			"@when (MeshType == Static) {\n"
			"	@input (mode = forward, id = \"vertex-position\") f32vec3 VertexPosition;\n"
			"	@input (mode = bind, id = \"view\", class = uniform) block VertexCamera { f32mat4 mvp; };\n"
			"	fn GetWorldPosition() -> f32vec4 { return f32vec4(VertexPosition, 1.0); }\n"
			"	@redefine (allow = true)\n"
			"	fn VertexOutputPosition() -> f32vec4 { return VertexCamera.mvp * GetWorldPosition(); }\n"
			"}\n";
		constexpr hg::StringView meshSkeletal =
			"#include <spec/defined.asl>\n"
			"@when (MeshType == Skeletal) {\n"
			"	@input (mode = forward, id = \"vertex-position\") f32vec3 VertexPosition;\n"
			"	@input (mode = forward, id = \"vertex-bone-idx\") u32 VertexBoneIdx;\n"
			"	@input (mode = bind, id = \"view\", class = uniform) block VertexCamera { f32mat4 mvp; };\n"
			"	@input (mode = bind, id = \"bones\", class = storage) block VertexBones { f32mat4[] list; };\n"
			"	fn GetWorldPosition() -> f32vec4 { return VertexBones.list[VertexBoneIdx] * f32vec4(VertexPosition, 1.0); }\n"
			"	@redefine (allow = true)\n"
			"	fn VertexOutputPosition() -> f32vec4 { return VertexCamera.mvp * GetWorldPosition(); }\n"
			"}\n";
		constexpr hg::StringView passVertexMain =
			"toggle PassContext : { DepthOnly, Material } = Material;\n"
			"@when (PassContext == Material) {\n"
			"	@output (mode = forward, id = \"world-position\") f32vec4 world_position;\n"
			"	fn main() { position = VertexOutputPosition(); world_position = GetWorldPosition(); }\n"
			"}\n"
			"@when (PassContext == DepthOnly)\n"
			"fn main() { position = VertexOutputPosition(); }\n";
	}

	TEST(IrSpirvGateTest, ToggleVariantsPerMeshTypeAndPassContext) {
		const auto source = hg::String { "#include <spec/defined.static.asl>\n#include <spec/defined.skeletal.asl>\n" } + hg::String { passVertexMain };
		for (const auto* mesh : { "Static", "Skeletal" }) {
			for (const auto* pass : { "DepthOnly", "Material" }) {
				EXPECT_TRUE(
					compilesToSpirv(
						source,
						EntryKind::eVertex,
						{ { "MeshType", mesh }, { "PassContext", pass } },
						{ { "spec/defined.asl", meshContract }, { "spec/defined.static.asl", meshStatic }, { "spec/defined.skeletal.asl", meshSkeletal } }
					)
				) << mesh << "/" << pass;
			}
		}
	}

	TEST(IrSpirvGateTest, ToggleVariantWithoutBodiesFails) {
		const auto source = hg::String { "#include <spec/defined.static.asl>\n" } + hg::String { passVertexMain };
		const auto result = compilesToSpirv(
			source,
			EntryKind::eVertex,
			{ { "MeshType", "Skeletal" } },
			{ { "spec/defined.asl", meshContract }, { "spec/defined.static.asl", meshStatic } }
		);
		EXPECT_FALSE(result);
		EXPECT_TRUE(std::string { result.message() }.contains("no body for")) << result.message();
	}

	TEST(IrSpirvGateTest, ConstantsSample) {
		EXPECT_TRUE(
			compilesToSpirv(
				"const SCALE: f32 = 0.5;\n"
				"const TABLE: f32vec3[2] = f32vec3[2](f32vec3(1.0), f32vec3(2.0));\n"
				"@spec (id = \"sample-count\") const SAMPLES: u32 = 4u;\n"
				"@input (mode = bind, id = \"m\", class = uniform, packing = std140) block m { f32vec4 a; };\n"
				"@output (mode = forward, id = \"color\") f32vec4 color;\n"
				"fn main() {\n"
				"	let acc = 0.0;\n"
				"	for (let i = 0u; i < SAMPLES; ++i) { acc += SCALE; }\n"
				"	color = f32vec4(TABLE[1] * acc, 1.0) + m.a;\n"
				"}\n",
				EntryKind::eFragment
			)
		);
	}

	TEST(IrSpirvGateTest, ReferencesSample) {
		EXPECT_TRUE(
			compilesToSpirv(
				"@input (mode = bind, id = \"lights\", class = storage, access = read_write) block lights { f32[] lights; };\n"
				"@output (mode = forward, id = \"color\") f32vec4 color;\n"
				"fn F(a: &mut f32, b: &f32, c: f32) { a = b + c; }\n"
				"fn main() {\n	u32 i = 1; let l = &mut lights.lights[i];\n	F(l, 1.0, 2.0);\n	const k = l;\n	color = f32vec4(k);\n}\n",
				EntryKind::eFragment
			)
		);
	}

	TEST(IrSpirvGateTest, ControlFlowSample) {
		EXPECT_TRUE(
			compilesToSpirv(
				"@output (mode = forward, id = \"color\") f32vec4 color;\n"
				"fn radical(bits_: u32) -> f32 {\n"
				"	let bits = bits_;\n"
				"	bits = (bits << 16u) | (bits >> 16u);\n"
				"	bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);\n"
				"	bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);\n"
				"	bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);\n"
				"	bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);\n"
				"	return f32(bits) * 2.3283064365386963e-10;\n"
				"}\n"
				"fn main() {\n"
				"	let acc = 0.0;\n"
				"	let n = 0u;\n"
				"	for (let i = 0u; i < 8u; ++i) { acc += radical(i); }\n"
				"	while (acc > 1.0) { acc -= 1.0; n++; if (n > 4u) { break; } if (acc > 100.0) { continue; } }\n"
				"	acc *= 0.5;\n"
				"	let r = acc > 0.5 ? acc : 1.0;\n"
				"	color = f32vec4(r, f32(n ^ 3u), f32(~n & 1u), 1.0);\n"
				"	if (r < 0.0) { discard; }\n"
				"}\n",
				EntryKind::eFragment
			)
		);
	}

	TEST(IrSpirvGateTest, MathSample) {
		EXPECT_TRUE(
			compilesToSpirv(
				"@output (mode = forward, id = \"color\") f32vec4 color;\n"
				"fn main() {\n"
				"	let v = f32vec3(0.25, 0.5, 0.75);\n"
				"	let m = f32mat4(1.0);\n"
				"	let n = f32mat3(m);\n"
				"	let t = transpose(inverse(n)) * normalize(clamp(v, 0.1, 0.9));\n"
				"	let a = mix(v, t, 0.5) + reflect(v, f32vec3(0.0, 1.0, 0.0)) + cross(v, t);\n"
				"	let b = dot(a, v) + length(a) + distance(a, v) + pow(abs(a.x), 2.0) + smoothstep(0.0, 1.0, a.y);\n"
				"	color = f32vec4(floor(a) + fract(a) + dFdx(a) + fwidth(a), sqrt(b) + sin(b) + max(b, 0.5));\n"
				"}\n",
				EntryKind::eFragment
			)
		);
	}

	TEST(IrSpirvGateTest, TextureSample) {
		EXPECT_TRUE(
			compilesToSpirv(
				"@input (mode = bind, id = \"t2\", class = texture) texture2d t2;\n"
				"@input (mode = bind, id = \"ta\", class = texture) texture2darray ta;\n"
				"@input (mode = bind, id = \"tc\", class = texture) texturecube tc;\n"
				"@input (mode = bind, id = \"smp\", class = sampler) sampler smp;\n"
				"@input (mode = forward, id = \"uv\") f32vec2 uv;\n"
				"@output (mode = forward, id = \"color\") f32vec4 color;\n"
				"fn main() {\n"
				"	let uv3 = f32vec3(uv, 0.0);\n"
				"	let a = f32vec4(0.0);\n"
				"	let b = f32vec4(0.0);\n"
				"	let acc = sample(t2, smp, uv) + sample(ta, smp, uv3) + sample(tc, smp, uv3);\n"
				"	acc += sample_lod(t2, smp, uv, 1.0) + sample_lod(ta, smp, uv3, 1.0) + sample_lod(tc, smp, uv3, 1.0);\n"
				"	let sz = texture_size(t2, smp, 0) + texture_size(tc, smp, 0) + s32vec2(texture_size(ta, smp, 0).xy);\n"
				"	let r = sample_sparse(t2, smp, uv, a) + sample_sparse(ta, smp, uv3, a) + sample_sparse(tc, smp, uv3, a);\n"
				"	let q = sample_sparse_clamp(t2, smp, uv, 1.0, b) + sample_sparse_clamp(ta, smp, uv3, 1.0, b) + sample_sparse_clamp(tc, smp, uv3, 1.0, b);\n"
				"	if (sparse_resident(r) && sparse_resident(q)) { acc += a + b; }\n"
				"	color = acc + f32vec4(f32(sz.x));\n"
				"}\n",
				EntryKind::eFragment
			)
		);
	}

	TEST(IrSpirvGateTest, ForWithOmittedInitAndStep) {
		EXPECT_TRUE(
			compilesToSpirv(
				"@output (mode = forward, id = \"color\") f32vec4 color;\n"
				"fn main() {\n"
				"	let acc = 0.0;\n"
				"	let i = 0u;\n"
				"	for (; i < 4u; ) { acc += 0.25; i++; }\n"
				"	for (let j = 0u; j < 2u; ) { acc += 0.5; j++; }\n"
				"	for (; acc < 8.0; ++acc) { }\n"
				"	color = f32vec4(acc);\n"
				"}\n",
				EntryKind::eFragment
			)
		);
	}

	TEST(IrSpirvGateTest, ComputeReduction) {
		EXPECT_TRUE(
			compilesToSpirv(
				"@input (mode = bind, id = \"data\", class = storage, access = read_write) block data { f32[] values; };\n"
				"workgroup scratch: f32[64];\n"
				"@compute (local_size = [64, 1, 1])\n"
				"fn main() {\n"
				"	let lid = local_invocation_index;\n"
				"	scratch[lid] = data.values[global_invocation_id.x];\n"
				"	barrier();\n"
				"	for (let stride = 32u; stride > 0u; stride = stride / 2u) {\n"
				"		if (lid < stride) { scratch[lid] += scratch[lid + stride]; }\n"
				"		barrier();\n"
				"	}\n"
				"	if (lid == 0u) { data.values[workgroup_id.x] = scratch[0]; }\n"
				"}\n",
				EntryKind::eCompute
			)
		);
	}

	TEST(IrSpirvGateTest, ConvolutionOverTileRegion) {
		EXPECT_TRUE(
			compilesToSpirv(
				"@input (mode = bind, id = \"result\", class = storage, access = read_write) block result { f32[] data; };\n"
				"workgroup tile: f32[324];\n"
				"@compute (local_size = [16, 16, 1])\n"
				"fn main() {\n"
				"	let x = local_invocation_id.x;\n"
				"	let row = local_invocation_id.y;\n"
				"	let base = (workgroup_id.x * 16u + row) * 16u;\n"
				"	tile[local_invocation_index] = 1.0;\n"
				"	barrier();\n"
				"	let src = &tile[row * 18u .. row * 18u + 18u];\n"
				"	let acc = src[x] + src[x + 1u] + src[x + 2u];\n"
				"	let dst = &mut result.data[base .. base + 16u];\n"
				"	dst[x] = acc;\n"
				"}\n",
				EntryKind::eCompute
			)
		);
	}

	TEST(IrSpirvGateTest, MicropolygonScratchpadPattern) {
		EXPECT_TRUE(
			compilesToSpirv(
				"@input (mode = bind, id = \"submit\", class = storage, access = read_write) block submit { u32[] data; };\n"
				"workgroup scratch: u32[1024];\n"
				"workgroup next: u32;\n"
				"fn write_index(r: &mut u32[], i: u32, v: u32) { r[i] = v; }\n"
				"fn write_vertex(r: &mut u32[], i: u32, v: u32) { r[i] = v + len(r); }\n"
				"@compute (local_size = [32, 1, 1])\n"
				"fn main() {\n"
				"	let slot = atomic_add(next, 1u);\n"
				"	const base: u32 = slot * 32u;\n"
				"	let lane = &mut scratch[base .. base + 32u];\n"
				"	const n: u32 = 12u;\n"
				"	let idx = &mut lane[0u .. n];\n"
				"	let vtx = &mut lane[n .. 32u];\n"
				"	write_index(idx, 0u, 1u);\n"
				"	write_index(idx, 1u, 2u);\n"
				"	write_vertex(vtx, 0u, 3u);\n"
				"	write_vertex(vtx, 1u, 4u);\n"
				"	const o: u32 = slot * 20u;\n"
				"	write_vertex(submit.data[o .. o + 20u], 0u, 5u);\n"
				"	const sbase: u32 = slot * 32u;\n"
				"	let slane = &mut submit.data[sbase .. sbase + 32u];\n"
				"	let sidx = &mut slane[0u .. n];\n"
				"	let svtx = &mut slane[n .. 32u];\n"
				"	write_index(sidx, 0u, 6u);\n"
				"	write_vertex(svtx, 0u, 7u);\n"
				"}\n",
				EntryKind::eCompute
			)
		);
	}

	TEST(IrSpirvGateTest, GeometryLayeredTriangle) {
		EXPECT_TRUE(
			compilesToSpirv(
				"@input (mode = forward, id = \"world-pos\") f32vec4[] p;\n"
				"@input (mode = forward, id = \"tint\") f32vec4[] tint_in;\n"
				"@output (mode = forward, id = \"tint-out\") f32vec4 tint_out;\n"
				"@geometry (input = triangles, output = triangle_strip, max_vertices = 3, invocations = 1)\n"
				"fn main() {\n"
				"	layer = 1;\n"
				"	for (let i = 0; i < 3; i++) {\n"
				"		position = input_position[i];\n"
				"		tint_out = tint_in[i] + p[i];\n"
				"		emit_vertex();\n"
				"	}\n"
				"	end_primitive();\n"
				"}\n",
				EntryKind::eGeometry
			)
		);
	}

	TEST(IrSpirvGateTest, TessControlTriangle) {
		EXPECT_TRUE(
			compilesToSpirv(
				"@input (mode = forward, id = \"world-pos\") f32vec4[] p;\n"
				"@output (mode = forward, id = \"ctrl-pos\") f32vec4[] q;\n"
				"@output (mode = forward, id = \"level\", patch = true) f32 lvl;\n"
				"@tess_ctrl (output_vertices = 3)\n"
				"fn main() {\n"
				"	q[invocation_id] = p[invocation_id];\n"
				"	output_position[invocation_id] = input_position[invocation_id];\n"
				"	barrier();\n"
				"	lvl = 2.0;\n"
				"	tess_level_outer[0] = 4.0;\n"
				"	tess_level_outer[1] = 4.0;\n"
				"	tess_level_outer[2] = 4.0;\n"
				"	tess_level_inner[0] = 4.0;\n"
				"}\n",
				EntryKind::eTessCtrl
			)
		);
	}

	TEST(IrSpirvGateTest, TessEvalTriangle) {
		EXPECT_TRUE(
			compilesToSpirv(
				"@input (mode = forward, id = \"ctrl-pos\") f32vec4[] q;\n"
				"@input (mode = forward, id = \"level\", patch = true) f32 lvl;\n"
				"@tess_eval (domain = triangles, spacing = equal, winding = ccw)\n"
				"fn main() {\n"
				"	position = (q[0] * tess_coord.x + q[1] * tess_coord.y + q[2] * tess_coord.z) * lvl;\n"
				"}\n",
				EntryKind::eTessEval
			)
		);
	}

	TEST(IrSpirvGateTest, ComputeImageBlur) {
		EXPECT_TRUE(
			compilesToSpirv(
				"@input (mode = bind, id = \"src\", class = image, access = read) image2d<rgba16f> src;\n"
				"@input (mode = bind, id = \"dst\", class = image, access = write) image2d<rgba16f> dst;\n"
				"@compute (local_size = [8, 8, 1])\n"
				"fn main() {\n"
				"	let p = s32vec2(global_invocation_id.xy);\n"
				"	let acc = f32vec4(0.0);\n"
				"	for (let dx = -1; dx <= 1; dx++) {\n"
				"		acc += image_load(src, p + s32vec2(dx, 0));\n"
				"	}\n"
				"	image_store(dst, p, acc / 3.0);\n"
				"}\n",
				EntryKind::eCompute
			)
		);
	}
}
