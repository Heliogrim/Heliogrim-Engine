#include "SkyboxModel.hpp"

#include <Engine.Assets.Type/Geometry/StaticGeometry.hpp>
#include <Engine.Assets.Type/Material/GfxMaterial.hpp>
#include <Engine.Common/Math/Coordinates.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.Reflect/Cast.hpp>
#include <Engine.Resource/ResourceManager.hpp>
#include <Heliogrim/Component/Scene/Sky/SkyboxComponent.hpp>

using namespace hg::engine::gfx;
using namespace hg;

SkyboxModel::SkyboxModel(const ptr<SceneComponent> owner_) :
	InheritMeta(owner_) {}

SkyboxModel::~SkyboxModel() {
	tidy();
}

void SkyboxModel::tidy() {}

void SkyboxModel::create(const ptr<render::RenderSceneSystem> system_) {

	auto origin = Cast<SkyboxComponent>(owner());

	//_boundary = origin->getBoundaries();
	if (origin->getSkyboxGeometry().isValid()) {
		_skyboxGeometryAsset = origin->getSkyboxGeometry().getAssetRef();
		_skyboxGeometryResource = Engine::getEngine()->getResources()->loader().load(std::addressof(*_skyboxGeometryAsset), nullptr);
	}

	if (origin->getSkyboxMaterial().isValid()) {
		const auto& materialAsset = *origin->getSkyboxMaterial().getAssetRef();
		auto materialResource = Engine::getEngine()->getResources()->loader().load(std::addressof(materialAsset), nullptr);

		_materials.emplace_back(materialResource.into<MaterialResource>());
	}
}

void SkyboxModel::update(const ptr<render::RenderSceneSystem> system_) {}

void SkyboxModel::destroy(const ptr<render::RenderSceneSystem> system_) {

	_skyboxGeometryResource.reset();
	_skyboxGeometryAsset = None;

	_materials.clear();
}

void SkyboxModel::capture(nmpt<render::MeshCaptureInterface> mci_) const noexcept {}

cref<smr<engine::resource::ResourceBase>> SkyboxModel::geometryResource() const noexcept {
	return _skyboxGeometryResource;
}

bool SkyboxModel::streamable() const noexcept {
	return _streamable;
}