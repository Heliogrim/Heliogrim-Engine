#include "BrdfBase.hpp"

#include <Engine.Common/Make.hpp>
#include <Engine.GFX.Render.Subpass/Impl/__tmp_helper.hpp>
#include <Engine.GFX/Geometry/Vertex.hpp>

using namespace ::hg::engine::render;
using namespace ::hg::engine::accel;
using namespace ::hg;

static smr<AccelerationEffect> build_test_effect() {

	const std::array vertexOffsets {
		std::pair<String, u32> { "vertex-position", static_cast<u32>(offsetof(engine::gfx::vertex, position)) },
		std::pair<String, u32> { "vertex-uv", static_cast<u32>(offsetof(engine::gfx::vertex, uvm)) },
		std::pair<String, u32> { "vertex-normal", static_cast<u32>(offsetof(engine::gfx::vertex, normal)) }
	};

	auto vertexStage = make_smr<Stage>(StageFlagBits::eVertex);
	vertexStage->setIntermediate(
		makeIrIntermediate(R"(brdf/brdf.vs.asl)", lang::ir::EntryKind::eVertex, vertexOffsets)
	);

	auto fragmentStage = make_smr<Stage>(StageFlagBits::eFragment);
	fragmentStage->setIntermediate(makeIrIntermediate(R"(brdf/brdf.fs.asl)", lang::ir::EntryKind::eFragment));

	/**/

	Guid guid { static_cast<u32>("__Test__Proxy"_typeId.data), 0, 0, "BrdfBaseEffect"_typeId.data };

	return make_smr<AccelerationEffect>(
		std::move(guid),
		"test-brdf-base-effect",
		Vector<smr<Stage>> { std::move(vertexStage), std::move(fragmentStage) }
	);
}

/**/

smr<const AccelerationEffect> engine::render::makeBrdfBaseEffect() {
	return build_test_effect();
}
