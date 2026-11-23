#include <Engine.Graphics.Render/RenderTarget.hpp>
#include <Engine.Graphics.Scene.Model/DirectionalLightModel.hpp>
#include <Engine.Graphics.Scene.Model/PointLightModel.hpp>
#include <Engine.Graphics.Scene.Model/SpotLightModel.hpp>
#include <Engine.Graphics.Scene.Model/StaticGeometryModel.hpp>
#include <Engine.Graphics.Scene.Model/Scene/SkyboxModel.hpp>

import Engine.Graphics.Components;
import Engine.Graphics.Scene;
import Engine.Reflow;
import Engine.Reflow.Components;
import Engine.Render.Scene;
import Engine.Render.Scene.Model;
import Engine.Scene;

using namespace hg::engine::graphics::scene;
using namespace hg::engine::graphics;
using namespace hg;

void RenderSceneManager::injectSceneHooks(nmpt<engine::scene::SceneBase> renderableScene_) {

	const auto sys = renderableScene_->getSceneSystem<render::RenderSceneSystem>();
	auto& rss = sys->getRegistry();

	/**/

	rss.addTypeMapping<CameraComponent, render::CameraModel>();

	/**/

	rss.addTypeMapping<StaticGeometryComponent, StaticGeometryModel>();
	rss.addTypeMapping<SkyboxComponent, SkyboxModel>();

	rss.addTypeMapping<PointLightComponent, PointLightModel>();
	rss.addTypeMapping<DirectionalLightComponent, DirectionalLightModel>();
	rss.addTypeMapping<SpotLightComponent, SpotLightModel>();

	/**/

	rss.addTypeMapping<UIComponent, reflow::UISceneModel>();
}
