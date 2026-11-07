#include "pch.h"

#include "IrSpirvGate.hpp"

using namespace hg::engine::accel::lang::ir;

namespace AccelCompile {
	TEST(IrSpirvGateTest, PreDepthVertex) {
		EXPECT_TRUE(assetCompilesToSpirv("depth/predepth.vs.asl", EntryKind::eVertex));
	}

	TEST(IrSpirvGateTest, PreDepthFragment) {
		EXPECT_TRUE(assetCompilesToSpirv("depth/predepth.fs.asl", EntryKind::eFragment));
	}

	TEST(IrSpirvGateTest, ShadowVertex) {
		EXPECT_TRUE(assetCompilesToSpirv("shadow/shadowDir.vs.asl", EntryKind::eVertex));
	}

	TEST(IrSpirvGateTest, ShadowFragment) {
		EXPECT_TRUE(assetCompilesToSpirv("shadow/shadowDir.fs.asl", EntryKind::eFragment));
	}

	TEST(IrSpirvGateTest, UiVertex) {
		EXPECT_TRUE(assetCompilesToSpirv("ui/ui.vs.asl", EntryKind::eVertex));
	}

	TEST(IrSpirvGateTest, UiFragment) {
		EXPECT_TRUE(assetCompilesToSpirv("ui/ui.fs.asl", EntryKind::eFragment));
	}

	TEST(IrSpirvGateTest, PostProcessVertex) {
		EXPECT_TRUE(assetCompilesToSpirv("postprocessing/pp.vs.asl", EntryKind::eVertex));
	}

	TEST(IrSpirvGateTest, PostProcessFragment) {
		EXPECT_TRUE(assetCompilesToSpirv("postprocessing/pp.fs.asl", EntryKind::eFragment));
	}

	TEST(IrSpirvGateTest, VisualizeVertex) {
		EXPECT_TRUE(assetCompilesToSpirv("visualize/visualize.vs.asl", EntryKind::eVertex));
	}

	TEST(IrSpirvGateTest, VisualizeFragment) {
		EXPECT_TRUE(assetCompilesToSpirv("visualize/visualize.fs.asl", EntryKind::eFragment));
	}

	TEST(IrSpirvGateTest, TriTestVertex) {
		EXPECT_TRUE(assetCompilesToSpirv("test/render.vs.asl", EntryKind::eVertex));
	}

	TEST(IrSpirvGateTest, TriTestFragment) {
		EXPECT_TRUE(assetCompilesToSpirv("test/render.fs.asl", EntryKind::eFragment));
	}

	TEST(IrSpirvGateTest, BrdfLutVertex) {
		EXPECT_TRUE(assetCompilesToSpirv("brdf/brdfLut.gen.vs.asl", EntryKind::eVertex));
	}

	TEST(IrSpirvGateTest, BrdfLutFragment) {
		EXPECT_TRUE(assetCompilesToSpirv("brdf/brdfLut.gen.fs.asl", EntryKind::eFragment));
	}

	TEST(IrSpirvGateTest, SkyVertex) {
		EXPECT_TRUE(assetCompilesToSpirv("skybox/sky.vs.asl", EntryKind::eVertex));
	}

	TEST(IrSpirvGateTest, SkyFragment) {
		EXPECT_TRUE(assetCompilesToSpirv("skybox/sky.fs.asl", EntryKind::eFragment));
	}

	TEST(IrSpirvGateTest, CubeGenVertex) {
		EXPECT_TRUE(assetCompilesToSpirv("brdf/cubeGen.vs.asl", EntryKind::eVertex));
	}

	TEST(IrSpirvGateTest, BrdfIrradFragment) {
		EXPECT_TRUE(assetCompilesToSpirv("brdf/brdfIrrad.gen.fs.asl", EntryKind::eFragment));
	}

	TEST(IrSpirvGateTest, BrdfPrefFragment) {
		EXPECT_TRUE(assetCompilesToSpirv("brdf/brdfPref.gen.fs.asl", EntryKind::eFragment));
	}

	TEST(IrSpirvGateTest, BrdfVertex) {
		EXPECT_TRUE(assetCompilesToSpirv("brdf/brdf.vs.asl", EntryKind::eVertex));
	}

	TEST(IrSpirvGateTest, BrdfFragment) {
		EXPECT_TRUE(assetCompilesToSpirv("brdf/brdf.fs.asl", EntryKind::eFragment));
	}

	TEST(IrSpirvGateTest, GBufferFragmentPerPassContext) {
		EXPECT_TRUE(assetCompilesToSpirv("brdf/gbuffer.fs.asl", EntryKind::eFragment, { { "PassContext", "DepthOnly" } }));
		EXPECT_TRUE(assetCompilesToSpirv("brdf/gbuffer.fs.asl", EntryKind::eFragment, { { "PassContext", "Material" } }));
	}

	TEST(IrSpirvGateTest, GBufferVertexPerMeshTypeAndPassContext) {
		for (const auto* mesh : { "Static", "Skeletal" }) {
			for (const auto* pass : { "DepthOnly", "Material" }) {
				EXPECT_TRUE(assetCompilesToSpirv("brdf/gbuffer.vs.asl", EntryKind::eVertex, { { "MeshType", mesh }, { "PassContext", pass } }))
					<< mesh << "/" << pass;
			}
		}
	}

	TEST(IrSpirvGateTest, RejectsBrokenGlsl) {
		EXPECT_FALSE(glslCompiles("#version 460\nvoid main(){ gl_Position = vec4(0); }", EntryKind::eFragment));
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

	/* Legacy glsl assets are removed from the asset checkout, so every remaining shader must be ASL. */
	TEST(IrSpirvGateTest, AllAssetsAreAsl) {
		namespace fs = std::filesystem;
		const auto root = fs::path { __FILE__ }.parent_path().parent_path().parent_path() / "assets" / "shader";

		std::string stale {};
		for (const auto& entry : fs::recursive_directory_iterator { root }) {
			if (not entry.is_regular_file()) {
				continue;
			}
			const auto name = entry.path().filename().string();
			if (entry.path().extension() != ".asl" && name != "compile_shaders.cmd") {
				stale += "\n  " + fs::relative(entry.path(), root).generic_string();
			}
		}
		EXPECT_TRUE(stale.empty()) << "Legacy shader files remain:" << stale;
	}

	TEST(IrSpirvGateTest, NoLegacyShaderLoads) {
		namespace fs = std::filesystem;
		const auto src = fs::path { __FILE__ }.parent_path().parent_path();

		/* split literals so this file does not match itself */
		const std::string loadNeedle = std::string { "read_shader" } + "_file(";
		const std::string macroNeedle = std::string { "__mac" } + "ros__";

		std::string hits {};
		size_t visited = 0uLL;
		for (const auto& entry : fs::recursive_directory_iterator { src }) {
			if (not entry.is_regular_file()) {
				continue;
			}
			const auto ext = entry.path().extension();
			if (ext != ".cpp" && ext != ".hpp") {
				continue;
			}
			++visited;
			const auto rel = fs::relative(entry.path(), src).generic_string();
			if (rel == "Engine.GFX.Render.Subpass/Impl/__tmp_helper.hpp") {
				continue;
			}
			/* defines its own unrelated loader for absolute paths from an old prototype */
			if (rel == "Engine.GFX.RenderGraph/test.cpp") {
				continue;
			}

			std::ifstream file { entry.path(), std::ios::in | std::ios::binary };
			std::ostringstream text {};
			text << file.rdbuf();
			const auto content = text.str();

			if (content.find(loadNeedle) != std::string::npos) {
				hits += "\n  " + rel + " (load)";
			}
			if (ext == ".cpp" && content.find(macroNeedle) != std::string::npos) {
				hits += "\n  " + rel + " (macros)";
			}
		}
		EXPECT_GT(visited, 0uLL) << "scan found no sources under " << src.string();
		EXPECT_TRUE(hits.empty()) << "Legacy shader references remain:" << hits;
	}
}
