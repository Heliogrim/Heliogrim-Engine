#include "SpotLightModel.hpp"

#include <cstring>
#include <Engine.Common/Math/Convertion.hpp>
#include <Engine.GFX/Light/Light.hpp>
#include <Engine.GFX/Pool/SceneResourcePool.hpp>
#include <Engine.Reflect/Cast.hpp>
#include <Engine.Render.Scene/RenderSceneSystem.hpp>
#include <Heliogrim/Component/Scene/Light/SpotLightComponent.hpp>

using namespace hg::engine::gfx::scene;
using namespace hg;

SpotLightModel::SpotLightModel(const ptr<SceneComponent> owner_) :
	InheritMeta(owner_),
	_sceneLightIndex() {}

SpotLightModel::~SpotLightModel() = default;

void SpotLightModel::create(const ptr<render::RenderSceneSystem> system_) {

	auto origin = Cast<SpotLightComponent>(owner());

	/**/

	const auto srp = system_->getSceneResourcePool();

	auto result = srp->lightSourcePool.acquire();
	_sceneLightIndex = result.instanceIndex;

	/**/

	auto& sceneLightInfo = srp->sceneLightInfo;
	if (sceneLightInfo.lightCount <= _sceneLightIndex) {
		sceneLightInfo.lightCount = _sceneLightIndex + 1uL;
		srp->sceneLightInfoBuffer.write<GlslSceneLightInfo>(&sceneLightInfo, 1uL);
	}

	/**/

	const GlslLight storeLight {
		// origin->_luminance,
		// _owner->getUniverseTransform().location().into(),
		// _owner->getUniverseTransform().rotator().euler(),
		// origin->_distanceLimit
	};

	// TODO: Spot-Light Cone
	origin->_innerCone;
	origin->_outerCone;

	/**/

	const auto page = result.dataView->pages().front();
	auto allocated = page->memory()->allocated();

	allocated->map(allocated->size);
	const auto innerOffset = result.dataView->offset() - page->resourceOffset();
	std::memcpy(static_cast<ptr<char>>(allocated->mapping) + innerOffset, &storeLight, sizeof(storeLight));
	allocated->unmap();

}

void SpotLightModel::update(const ptr<render::RenderSceneSystem> system_) {}

void SpotLightModel::destroy(const ptr<render::RenderSceneSystem> system_) {

	const auto srp = system_->getSceneResourcePool();
	srp->lightSourcePool.release(_sceneLightIndex);
}

void SpotLightModel::capture(nmpt<render::LightCaptureInterface> lci_) const noexcept {}
