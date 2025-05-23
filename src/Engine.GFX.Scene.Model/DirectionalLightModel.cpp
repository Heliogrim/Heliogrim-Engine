#include "DirectionalLightModel.hpp"

#include <cstring>
#include <Engine.Common/Collection/Array.hpp>
#include <Engine.Common/Math/Coordinates.hpp>
#include <Engine.GFX/Pool/SceneResourcePool.hpp>
#include <Engine.Reflect/Cast.hpp>
#include <Engine.Render.Scene/RenderSceneSystem.hpp>
#include <Heliogrim/Component/Scene/Light/DirectionalLightComponent.hpp>

using namespace hg::engine::gfx::scene;
using namespace hg;

DirectionalLightModel::DirectionalLightModel(const ptr<SceneComponent> owner_) :
	InheritMeta(owner_),
	_sceneLightIndex(),
	_sceneShadowIndex(~0) {}

DirectionalLightModel::~DirectionalLightModel() = default;

math::fvec3 DirectionalLightModel::getLightDirection() const noexcept {
	auto origin = Cast<DirectionalLightComponent>(owner());
	return origin->_direction.rotated(math::vec3_down).normalized();
}

void DirectionalLightModel::create(const ptr<render::RenderSceneSystem> system_) {

	auto origin = Cast<DirectionalLightComponent>(owner());
	const auto srp = system_->getSceneResourcePool();

	/**/

	{
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
		std::memcpy(static_cast<ptr<char>>(allocated->mapping) + innerOffset, &storeLight, sizeof(storeLight));
		allocated->unmap();
	}

	/**/

	{
		auto result = srp->shadowSourcePool.acquire();
		_sceneShadowIndex = result.instanceIndex;

		/**/

		const GlslDirectionalShadow storeShadow {
			{
				math::mat4::make_identity(),
				math::mat4::make_identity(),
				math::mat4::make_identity(),
				math::mat4::make_identity()
			}
		};

		/**/

		const auto offset = sizeof(GlslDirectionalShadow) * result.instanceIndex;
		result.dataView.map(sizeof(GlslDirectionalShadow), offset);
		result.dataView.write<GlslDirectionalShadow>(&storeShadow, 1uL);
		result.dataView.unmap();
	}

}

void DirectionalLightModel::update(const ptr<render::RenderSceneSystem> system_) {

	auto origin = Cast<DirectionalLightComponent>(owner());

	const auto srp = system_->getSceneResourcePool();

	/**/

	const GlslLight storeLight {
		math::fvec4 { origin->_luminance, 0.F },
		math::fvec4 { math::vec3_zero, 0.F },
		math::fvec4 { origin->_direction.rotated(math::vec3_down), 0.F },
		0.F
	};

	/**/

	const auto view = srp->lightSourcePool.getDataView(_sceneLightIndex);
	const auto page = view->pages().front();
	auto allocated = page->memory()->allocated();

	allocated->map(allocated->size);
	const auto innerOffset = view->offset() - page->resourceOffset();
	std::memcpy(static_cast<ptr<char>>(allocated->mapping) + innerOffset, &storeLight, sizeof(storeLight));
	allocated->unmap();
}

void DirectionalLightModel::destroy(const ptr<render::RenderSceneSystem> system_) {

	const auto srp = system_->getSceneResourcePool();
	srp->lightSourcePool.release(_sceneLightIndex);
	srp->shadowSourcePool.release(_sceneShadowIndex);
}

void DirectionalLightModel::capture(nmpt<render::LightCaptureInterface> lci_) const noexcept {}
