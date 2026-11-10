#include "Skybox.hpp"

#include <Engine.Common/Make.hpp>
#include <Engine.Graphics.Render.Subpass/Impl/__tmp_helper.hpp>

#include "Engine.Graphics.Render.Predefined/Symbols/SceneCamera.hpp"
#include "Engine.Graphics.Render.Predefined/Symbols/SceneColor.hpp"
#include "Engine.Graphics.Render.Predefined/Symbols/SkyboxTexture.hpp"

using namespace hg::engine::render;
using namespace hg;
using namespace ::hg::engine::accel;

static smr<AccelerationEffect> build_test_effect() {

	auto vertexStage = make_smr<Stage>(StageFlagBits::eVertex);
	vertexStage->setIntermediate(makeIrIntermediate(R"(skybox/sky.vs.asl)", lang::ir::EntryKind::eVertex));

	auto fragmentStage = make_smr<Stage>(StageFlagBits::eFragment);
	fragmentStage->setIntermediate(makeIrIntermediate(R"(skybox/sky.fs.asl)", lang::ir::EntryKind::eFragment));

	/**/

	Guid guid { static_cast<u32>("__Test__Proxy"_typeId.data), 0, 0, "SkyboxEffect"_typeId.data };

	return make_smr<AccelerationEffect>(
		std::move(guid),
		"test-sky-effect",
		Vector<smr<Stage>> { std::move(vertexStage), std::move(fragmentStage) }
	);
	// Vector<smr<const Symbol>> { makeSceneCameraSymbol(), makeSkyboxTextureSymbol(), /*makeSceneDepthSymbol()*/ },
	// Vector<smr<const Symbol>> { makeSceneColorSymbol() }
}

/**/

smr<const AccelerationEffect> engine::render::makeSkyboxEffect() {
	return build_test_effect();
}
