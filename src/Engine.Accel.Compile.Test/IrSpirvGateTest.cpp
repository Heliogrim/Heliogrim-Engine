#include "pch.h"

/**/

#include <filesystem>
#include <ranges>
#include <regex>

#include "IrSpirvGate.hpp"

using namespace hg::engine::accel::lang::ir;

namespace AccelCompile {
	TEST(IrSpirvGateTest, RejectsMissingOrBrokenSpirv) {
		hg::engine::accel::lang::Intermediate im {};
		EXPECT_FALSE(spirvValid(im));
		/* header only: no capability, no memory model */
		im.spirv = { 0x07230203uL, 0x00010000uL, 0uL, 1uL, 0uL };
		EXPECT_FALSE(spirvValid(im));
	}

	TEST(IrSpirvGateTest, InterfaceDescPerEntry) {
		const std::array stages {
			GateStage {
				"@input (mode = forward, id = \"vertex-position\") f32vec3 local_position;\n"
				"@output (mode = forward, id = \"uv\") f32vec2 uv;\n"
				"fn main() {\n\tuv = local_position.xy;\n\tposition = f32vec4(local_position, 1.0);\n}\n",
				EntryKind::eVertex
			},
			GateStage {
				"@input (mode = forward, id = \"uv\") f32vec2 uv;\n"
				"@output (mode = forward, id = \"color\") f32vec4 color;\n"
				"fn main() {\n\tcolor = f32vec4(uv, 0.0, 1.0);\n}\n",
				EntryKind::eFragment
			}
		};
		const auto composed = composeStages(stages, {}, GateResolver { [](hg::StringView) -> hg::Opt<hg::String> { return hg::None; } });
		ASSERT_TRUE(composed.has_value()) << composed.error();
		ASSERT_EQ(composed->size(), 2uz);

		const auto& vertex = *(*composed)[0]->getIntermediate();
		const auto& fragment = *(*composed)[1]->getIntermediate();
		EXPECT_TRUE(spirvValid(vertex));
		EXPECT_TRUE(spirvValid(fragment));
		EXPECT_EQ(vertex.interface.stage, EntryKind::eVertex);
		EXPECT_EQ(fragment.interface.stage, EntryKind::eFragment);

		const auto inputs = fragment.interface.entries | std::views::filter([](const auto& e_) { return e_.input; }) | std::ranges::to<
			hg::Vector<InterfaceEntry>>();
		ASSERT_EQ(inputs.size(), 1uz);
		EXPECT_EQ(inputs.front().id, "uv");
		EXPECT_EQ(inputs.front().mode, BindMode::eForward);
		EXPECT_TRUE(std::ranges::any_of(fragment.interface.entries, [](const auto& e_) { return not e_.input and e_.id == "color"; }));
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
				"\tlet m = model.model[instance_index];\n"
				"\tlet shift = f32vec2(0.0);\n"
				"\tif (vertex_index == 0) { shift = push.translate; }\n"
				"\tlet pos = m * f32vec4(local_position + f32vec3(shift, 0.0), 1.0);\n"
				"\tposition = cascades.viewProj[push.cascade] * camera.mvp * pos;\n"
				"\tworld_normal = f32vec4(normalize(transpose(inverse(f32mat3(m))) * local_normal), 0.0);\n"
				"\tout_uv = UVS[vertex_index];\n"
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
				"\treturn l.luminance.rgb * saturate(1.0 - length(l.position.xyz - p) / l.limit);\n"
				"}\n"
				"fn main() {\n"
				"\tlet eye = inverse(camera.view)[3].xyz;\n"
				"\tlet tndc = f32mat4(0.5, 0.0, 0.0, 0.0, 0.0, 0.5, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.5, 0.5, 0.0, 1.0);\n"
				"\tlet lo = f32vec3(0.0);\n"
				"\tfor (let i = 0u; i < info.count; ++i) {\n"
				"\t\tlet l = &sl.lights[i];\n"
				"\t\tif (l.limit > 0.0) { lo += falloff(l, world_position.xyz); } else { lo += l.luminance.rgb; }\n"
				"\t}\n"
				"\tlet s = tndc * shadows.shadows[0].viewProj[1] * world_position;\n"
				"\tlet d = sample(shadow_dir, shadow_smp, f32vec3(s.xy / s.w, 1.0)).r;\n"
				"\tlet shade = s.z > d ? 0.07 : 1.0;\n"
				"\tcolor = f32vec4(saturate(lo * shade) / max(length(eye - world_position.xyz), 1.0), PI);\n"
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
			"\t@input (mode = forward, id = \"vertex-position\") f32vec3 VertexPosition;\n"
			"\t@input (mode = bind, id = \"view\", class = uniform) block VertexCamera { f32mat4 mvp; };\n"
			"\tfn GetWorldPosition() -> f32vec4 { return f32vec4(VertexPosition, 1.0); }\n"
			"\t@redefine (allow = true)\n"
			"\tfn VertexOutputPosition() -> f32vec4 { return VertexCamera.mvp * GetWorldPosition(); }\n"
			"}\n";
		constexpr hg::StringView meshSkeletal =
			"#include <spec/defined.asl>\n"
			"@when (MeshType == Skeletal) {\n"
			"\t@input (mode = forward, id = \"vertex-position\") f32vec3 VertexPosition;\n"
			"\t@input (mode = forward, id = \"vertex-bone-idx\") u32 VertexBoneIdx;\n"
			"\t@input (mode = bind, id = \"view\", class = uniform) block VertexCamera { f32mat4 mvp; };\n"
			"\t@input (mode = bind, id = \"bones\", class = storage) block VertexBones { f32mat4[] list; };\n"
			"\tfn GetWorldPosition() -> f32vec4 { return VertexBones.list[VertexBoneIdx] * f32vec4(VertexPosition, 1.0); }\n"
			"\t@redefine (allow = true)\n"
			"\tfn VertexOutputPosition() -> f32vec4 { return VertexCamera.mvp * GetWorldPosition(); }\n"
			"}\n";
		constexpr hg::StringView passVertexMain =
			"toggle PassContext : { DepthOnly, Material } = Material;\n"
			"@when (PassContext == Material) {\n"
			"\t@output (mode = forward, id = \"world-position\") f32vec4 world_position;\n"
			"\tfn main() { position = VertexOutputPosition(); world_position = GetWorldPosition(); }\n"
			"}\n"
			"@when (PassContext == DepthOnly)\n"
			"fn main() { position = VertexOutputPosition(); }\n";
	}

	TEST(IrSpirvGateTest, ToggleVariantsPerMeshTypeAndPassContext) {
		const auto source = hg::String { "#include <spec/defined.static.asl>\n#include <spec/defined.skeletal.asl>\n" } + hg::String {
			passVertexMain
		};
		for (const auto* mesh : { "Static", "Skeletal" }) {
			for (const auto* pass : { "DepthOnly", "Material" }) {
				EXPECT_TRUE(
					compilesToSpirv(
						source,
						EntryKind::eVertex,
						{ { "MeshType", mesh }, { "PassContext", pass } },
						{ { "spec/defined.asl", meshContract }, { "spec/defined.static.asl", meshStatic }, { "spec/defined.skeletal.asl",
							meshSkeletal } }
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
				"\tlet acc = 0.0;\n"
				"\tfor (let i = 0u; i < SAMPLES; ++i) { acc += SCALE; }\n"
				"\tcolor = f32vec4(TABLE[1] * acc, 1.0) + m.a;\n"
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
				"fn main() {\n\tu32 i = 1; let l = &mut lights.lights[i];\n\tF(l, 1.0, 2.0);\n\tconst k = l;\n\tcolor = f32vec4(k);\n}\n",
				EntryKind::eFragment
			)
		);
	}

	TEST(IrSpirvGateTest, ControlFlowSample) {
		EXPECT_TRUE(
			compilesToSpirv(
				"@output (mode = forward, id = \"color\") f32vec4 color;\n"
				"fn radical(bits_: u32) -> f32 {\n"
				"\tlet bits = bits_;\n"
				"\tbits = (bits << 16u) | (bits >> 16u);\n"
				"\tbits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);\n"
				"\tbits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);\n"
				"\tbits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);\n"
				"\tbits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);\n"
				"\treturn f32(bits) * 2.3283064365386963e-10;\n"
				"}\n"
				"fn main() {\n"
				"\tlet acc = 0.0;\n"
				"\tlet n = 0u;\n"
				"\tfor (let i = 0u; i < 8u; ++i) { acc += radical(i); }\n"
				"\twhile (acc > 1.0) { acc -= 1.0; n++; if (n > 4u) { break; } if (acc > 100.0) { continue; } }\n"
				"\tacc *= 0.5;\n"
				"\tlet r = acc > 0.5 ? acc : 1.0;\n"
				"\tcolor = f32vec4(r, f32(n ^ 3u), f32(~n & 1u), 1.0);\n"
				"\tif (r < 0.0) { discard; }\n"
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
				"\tlet v = f32vec3(0.25, 0.5, 0.75);\n"
				"\tlet m = f32mat4(1.0);\n"
				"\tlet n = f32mat3(m);\n"
				"\tlet t = transpose(inverse(n)) * normalize(clamp(v, 0.1, 0.9));\n"
				"\tlet a = mix(v, t, 0.5) + reflect(v, f32vec3(0.0, 1.0, 0.0)) + cross(v, t);\n"
				"\tlet b = dot(a, v) + length(a) + distance(a, v) + pow(abs(a.x), 2.0) + smoothstep(0.0, 1.0, a.y);\n"
				"\tcolor = f32vec4(floor(a) + fract(a) + dFdx(a) + fwidth(a), sqrt(b) + sin(b) + max(b, 0.5));\n"
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
				"\tlet uv3 = f32vec3(uv, 0.0);\n"
				"\tlet a = f32vec4(0.0);\n"
				"\tlet b = f32vec4(0.0);\n"
				"\tlet acc = sample(t2, smp, uv) + sample(ta, smp, uv3) + sample(tc, smp, uv3);\n"
				"\tacc += sample_lod(t2, smp, uv, 1.0) + sample_lod(ta, smp, uv3, 1.0) + sample_lod(tc, smp, uv3, 1.0);\n"
				"\tlet sz = texture_size(t2, smp, 0) + texture_size(tc, smp, 0) + s32vec2(texture_size(ta, smp, 0).xy);\n"
				"\tlet r = sample_sparse(t2, smp, uv, a) + sample_sparse(ta, smp, uv3, a) + sample_sparse(tc, smp, uv3, a);\n"
				"\tlet q = sample_sparse_clamp(t2, smp, uv, 1.0, b) + sample_sparse_clamp(ta, smp, uv3, 1.0, b) + sample_sparse_clamp(tc, smp, uv3, 1.0, b);\n"
				"\tif (sparse_resident(r) && sparse_resident(q)) { acc += a + b; }\n"
				"\tcolor = acc + f32vec4(f32(sz.x));\n"
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
				"\tlet acc = 0.0;\n"
				"\tlet i = 0u;\n"
				"\tfor (; i < 4u; ) { acc += 0.25; i++; }\n"
				"\tfor (let j = 0u; j < 2u; ) { acc += 0.5; j++; }\n"
				"\tfor (; acc < 8.0; ++acc) { }\n"
				"\tcolor = f32vec4(acc);\n"
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
				"\tlet lid = local_invocation_index;\n"
				"\tscratch[lid] = data.values[global_invocation_id.x];\n"
				"\tbarrier();\n"
				"\tfor (let stride = 32u; stride > 0u; stride = stride / 2u) {\n"
				"\t\tif (lid < stride) { scratch[lid] += scratch[lid + stride]; }\n"
				"\t\tbarrier();\n"
				"\t}\n"
				"\tif (lid == 0u) { data.values[workgroup_id.x] = scratch[0]; }\n"
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
				"\tlet x = local_invocation_id.x;\n"
				"\tlet row = local_invocation_id.y;\n"
				"\tlet base = (workgroup_id.x * 16u + row) * 16u;\n"
				"\ttile[local_invocation_index] = 1.0;\n"
				"\tbarrier();\n"
				"\tlet src = &tile[row * 18u .. row * 18u + 18u];\n"
				"\tlet acc = src[x] + src[x + 1u] + src[x + 2u];\n"
				"\tlet dst = &mut result.data[base .. base + 16u];\n"
				"\tdst[x] = acc;\n"
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
				"\tlet slot = atomic_add(next, 1u);\n"
				"\tconst base: u32 = slot * 32u;\n"
				"\tlet lane = &mut scratch[base .. base + 32u];\n"
				"\tconst n: u32 = 12u;\n"
				"\tlet idx = &mut lane[0u .. n];\n"
				"\tlet vtx = &mut lane[n .. 32u];\n"
				"\twrite_index(idx, 0u, 1u);\n"
				"\twrite_index(idx, 1u, 2u);\n"
				"\twrite_vertex(vtx, 0u, 3u);\n"
				"\twrite_vertex(vtx, 1u, 4u);\n"
				"\tconst o: u32 = slot * 20u;\n"
				"\twrite_vertex(submit.data[o .. o + 20u], 0u, 5u);\n"
				"\tconst sbase: u32 = slot * 32u;\n"
				"\tlet slane = &mut submit.data[sbase .. sbase + 32u];\n"
				"\tlet sidx = &mut slane[0u .. n];\n"
				"\tlet svtx = &mut slane[n .. 32u];\n"
				"\twrite_index(sidx, 0u, 6u);\n"
				"\twrite_vertex(svtx, 0u, 7u);\n"
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
				"\tlayer = 1;\n"
				"\tfor (let i = 0; i < 3; i++) {\n"
				"\t\tposition = input_position[i];\n"
				"\t\ttint_out = tint_in[i] + p[i];\n"
				"\t\temit_vertex();\n"
				"\t}\n"
				"\tend_primitive();\n"
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
				"\tq[invocation_id] = p[invocation_id];\n"
				"\toutput_position[invocation_id] = input_position[invocation_id];\n"
				"\tbarrier();\n"
				"\tlvl = 2.0;\n"
				"\ttess_level_outer[0] = 4.0;\n"
				"\ttess_level_outer[1] = 4.0;\n"
				"\ttess_level_outer[2] = 4.0;\n"
				"\ttess_level_inner[0] = 4.0;\n"
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
				"\tposition = (q[0] * tess_coord.x + q[1] * tess_coord.y + q[2] * tess_coord.z) * lvl;\n"
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
				"\tlet p = s32vec2(global_invocation_id.xy);\n"
				"\tlet acc = f32vec4(0.0);\n"
				"\tfor (let dx = -1; dx <= 1; dx++) {\n"
				"\t\tacc += image_load(src, p + s32vec2(dx, 0));\n"
				"\t}\n"
				"\timage_store(dst, p, acc / 3.0);\n"
				"}\n",
				EntryKind::eCompute
			)
		);
	}

	TEST(IrSpirvGateTest, EngineShaders) {
		const GateResolver resolver { readShader };
		const auto shaders = engineShaders();
		ASSERT_FALSE(shaders.empty());
		for (const auto& [name, entry] : shaders) {
			const auto source = readShader(name);
			const std::array stages { GateStage { *source, entry } };
			const auto composed = composeStages(stages, {}, resolver);
			ASSERT_TRUE(composed.has_value()) << name << ": " << composed.error();
			EXPECT_TRUE(spirvValid(*composed->front()->getIntermediate())) << name;
		}
	}
}
