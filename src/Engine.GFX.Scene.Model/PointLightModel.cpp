#include "PointLightModel.hpp"

#include <Engine.Common/Math/Convertion.hpp>
#include <Engine.Common/Math/Coordinates.hpp>
#include <Engine.GFX/Light/Light.hpp>
#include <Engine.GFX/Pool/SceneResourcePool.hpp>
#include <Engine.Scene/RevScene.hpp>
#include <Heliogrim/Components/PointLightComponent.hpp>

using namespace hg::engine::gfx::scene;
using namespace hg;

PointLightModel::PointLightModel(const ptr<SceneComponent> owner_) :
    InheritMeta(owner_),
    _sceneLightIndex() {}

PointLightModel::~PointLightModel() {}

void PointLightModel::create(const ptr<engine::scene::Scene> scene_) {

    const auto* const origin = static_cast<ptr<PointLightComponent>>(_owner);

    /**/

    const auto* const rscene = static_cast<const ptr<engine::scene::RevScene>>(scene_);

    auto srp = rscene->getSceneResourcePool();
    const auto result = srp->lightSourcePool.acquire();
    _sceneLightIndex = result.instanceIndex;

    /**/

    auto& sceneLightInfo = srp->sceneLightInfo;
    if (sceneLightInfo.lightCount <= _sceneLightIndex) {
        sceneLightInfo.lightCount = _sceneLightIndex + 1uL;
        srp->sceneLightInfoBuffer.write<GlslSceneLightInfo>(&sceneLightInfo, 1uL);
    }

    /**/

    const GlslLight storeLight {
        math::fvec4 { origin->_luminance, 0.F },
        math::fvec4 { _owner->getWorldTransform().location().fvec3(), 0.F },
        math::fvec4 { math::vec3_zero, 0.F },
        origin->_distanceLimit
    };

    /**/

    const auto* const page = result.dataView->pages().front();
    auto allocated = page->memory()->allocated();

    allocated->map(allocated->size);
    const auto innerOffset = result.dataView->offset() - page->resourceOffset();
    memcpy(static_cast<ptr<char>>(allocated->mapping) + innerOffset, &storeLight, sizeof(storeLight));
    allocated->unmap();

}

void PointLightModel::update(const ptr<engine::scene::Scene> scene_) {

    const auto* const origin = static_cast<ptr<PointLightComponent>>(_owner);

    const auto* const rscene = static_cast<const ptr<engine::scene::RevScene>>(scene_);
    auto srp = rscene->getSceneResourcePool();

    /**/

    const GlslLight storeLight {
        math::fvec4 { origin->_luminance, 0.F },
        math::fvec4 { _owner->getWorldTransform().location().fvec3(), 0.F },
        math::fvec4 { math::vec3_zero, 0.F },
        origin->_distanceLimit
    };

    /**/

    const auto view = srp->lightSourcePool.getDataView(_sceneLightIndex);
    const auto* const page = view->pages().front();
    auto allocated = page->memory()->allocated();

    allocated->map(allocated->size);
    const auto innerOffset = view->offset() - page->resourceOffset();
    memcpy(static_cast<ptr<char>>(allocated->mapping) + innerOffset, &storeLight, sizeof(storeLight));
    allocated->unmap();
}

void PointLightModel::destroy(const ptr<engine::scene::Scene> scene_) {

    const auto* const rscene = static_cast<const ptr<engine::scene::RevScene>>(scene_);

    auto srp = rscene->getSceneResourcePool();
    srp->lightSourcePool.release(_sceneLightIndex);
}

void PointLightModel::capture(nmpt<render::LightCaptureInterface> lci_) const noexcept {}
