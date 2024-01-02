#include "DirectionalLightModel.hpp"

#include <Engine.Common/Math/Coordinates.hpp>
#include <Engine.GFX/Pool/SceneResourcePool.hpp>
#include <Engine.Scene/RevScene.hpp>
#include <Heliogrim/Components/DirectionalLightComponent.hpp>

using namespace hg::engine::gfx::scene;
using namespace hg;

DirectionalLightModel::DirectionalLightModel(const ptr<SceneComponent> owner_) :
    InheritMeta(owner_),
    _sceneLightIndex() {}

DirectionalLightModel::~DirectionalLightModel() = default;

math::fvec3 DirectionalLightModel::getLightDirection() const noexcept {
    const auto* const origin = static_cast<ptr<DirectionalLightComponent>>(_owner);
    return origin->_direction.rotated(math::vec3_down);
}

void DirectionalLightModel::create(const ptr<engine::scene::Scene> scene_) {

    const auto* const origin = static_cast<ptr<DirectionalLightComponent>>(_owner);

    /**/

    auto rscene = static_cast<const ptr<engine::scene::RevScene>>(scene_);

    auto srp = rscene->getSceneResourcePool();
    auto result = srp->lightSourcePool.acquire();
    _sceneLightIndex = result.instanceIndex;

    /**/

    const GlslLight storeLight {
        math::fvec4 { origin->_luminance, 0.F },
        math::fvec4 { math::vec3_zero, 0.F },
        math::fvec4 { origin->_direction.rotated(math::vec3_down), 0.F },
        0.F
    };

    /**/

    const auto page = result.dataView->pages().front();
    auto allocated = page->memory()->allocated();

    allocated->map(allocated->size);
    const auto innerOffset = result.dataView->offset() - page->resourceOffset();
    memcpy(static_cast<ptr<char>>(allocated->mapping) + innerOffset, &storeLight, sizeof(storeLight));
    allocated->unmap();

}

void DirectionalLightModel::update(const ptr<engine::scene::Scene> scene_) {

    const auto* const origin = static_cast<ptr<DirectionalLightComponent>>(_owner);

    const auto* const rscene = static_cast<const ptr<engine::scene::RevScene>>(scene_);
    auto srp = rscene->getSceneResourcePool();

    /**/

    const GlslLight storeLight {
        math::fvec4 { origin->_luminance, 0.F },
        math::fvec4 { math::vec3_zero, 0.F },
        math::fvec4 { origin->_direction.rotated(math::vec3_down), 0.F },
        0.F
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

void DirectionalLightModel::destroy(const ptr<engine::scene::Scene> scene_) {

    auto rscene = static_cast<const ptr<engine::scene::RevScene>>(scene_);

    auto srp = rscene->getSceneResourcePool();
    srp->lightSourcePool.release(_sceneLightIndex);
}

void DirectionalLightModel::capture(nmpt<render::LightCaptureInterface> lci_) const noexcept {}
