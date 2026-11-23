#include "SkyboxModel.hpp"

#include <Engine.Graphics.Components/Sky/SkyboxComponent.hpp>

import Engine.Assets.Type;
import Engine.Common;
import Engine.Core;
import Engine.Reflect;
import Engine.Resource;

using namespace hg::engine::graphics;
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
