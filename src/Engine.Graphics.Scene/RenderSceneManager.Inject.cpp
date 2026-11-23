#include <Engine.Graphics/RenderTarget.hpp>
#include <Engine.Graphics/Scene/SkyboxModel.hpp>
#include <Engine.Graphics.Components/Camera/CameraComponent.hpp>
#include <Engine.Graphics.Components/Geometry/StaticGeometryComponent.hpp>
#include <Engine.Graphics.Components/Light/DirectionalLightComponent.hpp>
#include <Engine.Graphics.Components/Light/PointLightComponent.hpp>
#include <Engine.Graphics.Components/Light/SpotLightComponent.hpp>
#include <Engine.Graphics.Components/Sky/SkyboxComponent.hpp>
#include <Engine.Graphics.Scene.Model/DirectionalLightModel.hpp>
#include <Engine.Graphics.Scene.Model/PointLightModel.hpp>
#include <Engine.Graphics.Scene.Model/SpotLightModel.hpp>
#include <Engine.Graphics.Scene.Model/StaticGeometryModel.hpp>
#include <Engine.Reflow/Scene/UISceneModel.hpp>
#include <Engine.Reflow.Components/UI/UIComponent.hpp>
#include <Engine.Render.Scene/RenderSceneSystem.hpp>
#include <Engine.Render.Scene.Model/CameraModel.hpp>

import Engine.Graphics.Scene;
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
