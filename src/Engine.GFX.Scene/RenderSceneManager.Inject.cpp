#include <Engine.GFX/Scene/SkyboxModel.hpp>
#include <Engine.GFX.Scene.Model/DirectionalLightModel.hpp>
#include <Engine.GFX.Scene.Model/PointLightModel.hpp>
#include <Engine.GFX.Scene.Model/SpotLightModel.hpp>
#include <Engine.GFX.Scene.Model/StaticGeometryModel.hpp>
#include <Engine.Reflow/Scene/UISceneModel.hpp>
#include <Engine.Render.Scene.Model/CameraModel.hpp>
#include <Engine.Render.Scene/RenderSceneSystem.hpp>
#include <Engine.Scene/SceneBase.hpp>
#include <Heliogrim/SkyboxComponent.hpp>
#include <Heliogrim/StaticGeometryComponent.hpp>
#include <Heliogrim/UIComponent.hpp>
#include <Heliogrim/Components/CameraComponent.hpp>
#include <Heliogrim/Components/DirectionalLightComponent.hpp>
#include <Heliogrim/Components/PointLightComponent.hpp>
#include <Heliogrim/Components/SpotLightComponent.hpp>

#include "RenderSceneManager.hpp"

using namespace hg::engine::gfx::scene;
using namespace hg::engine::gfx;
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
