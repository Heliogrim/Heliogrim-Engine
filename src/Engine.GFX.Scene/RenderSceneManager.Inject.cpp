#include <Engine.GFX/Scene/SkyboxModel.hpp>
#include <Engine.GFX.Scene.Model/DirectionalLightModel.hpp>
#include <Engine.GFX.Scene.Model/PointLightModel.hpp>
#include <Engine.GFX.Scene.Model/SpotLightModel.hpp>
#include <Engine.GFX.Scene.Model/StaticGeometryModel.hpp>
#include <Engine.GFX/RenderTarget.hpp>
#include <Engine.Reflow/Scene/UISceneModel.hpp>
#include <Engine.Render.Scene.Model/CameraModel.hpp>
#include <Engine.Render.Scene/RenderSceneSystem.hpp>
#include <Engine.Scene/SceneBase.hpp>
#include <Heliogrim/Component/Scene/Camera/CameraComponent.hpp>
#include <Heliogrim/Component/Scene/Geometry/StaticGeometryComponent.hpp>
#include <Heliogrim/Component/Scene/Light/DirectionalLightComponent.hpp>
#include <Heliogrim/Component/Scene/Light/PointLightComponent.hpp>
#include <Heliogrim/Component/Scene/Light/SpotLightComponent.hpp>
#include <Heliogrim/Component/Scene/Sky/SkyboxComponent.hpp>
#include <Heliogrim/Component/Scene/UI/UIComponent.hpp>

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
