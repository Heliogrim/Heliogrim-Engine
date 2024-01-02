#include "SpotLightModel.hpp"

#include <Engine.Common/Math/Convertion.hpp>
#include <Engine.GFX/Light/Light.hpp>
#include <Engine.GFX/Pool/SceneResourcePool.hpp>
#include <Engine.Scene/RevScene.hpp>
#include <Heliogrim/Components/SpotLightComponent.hpp>

using namespace hg::engine::gfx::scene;
using namespace hg;

SpotLightModel::SpotLightModel(const ptr<SceneComponent> owner_) :
    InheritMeta(owner_),
    _sceneLightIndex() {}

SpotLightModel::~SpotLightModel() = default;

void SpotLightModel::create(const ptr<engine::scene::Scene> scene_) {

    const auto* const origin = static_cast<ptr<SpotLightComponent>>(_owner);

    /**/

    auto rscene = static_cast<const ptr<engine::scene::RevScene>>(scene_);

    auto srp = rscene->getSceneResourcePool();
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
        // _owner->getWorldTransform().location().fvec3(),
        // _owner->getWorldTransform().rotator().euler(),
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
    memcpy(static_cast<ptr<char>>(allocated->mapping) + innerOffset, &storeLight, sizeof(storeLight));
    allocated->unmap();

}

void SpotLightModel::update(const ptr<engine::scene::Scene> scene_) {}

void SpotLightModel::destroy(const ptr<engine::scene::Scene> scene_) {

    const auto* const rscene = static_cast<const ptr<engine::scene::RevScene>>(scene_);

    auto srp = rscene->getSceneResourcePool();
    srp->lightSourcePool.release(_sceneLightIndex);
}

void SpotLightModel::capture(nmpt<render::LightCaptureInterface> lci_) const noexcept {}
