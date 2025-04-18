#include "SkyboxModel.hpp"

#include <Engine.Common/Math/Coordinates.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.Reflect/Cast.hpp>
#include <Engine.Resource/ResourceManager.hpp>
#include <Heliogrim/Component/Scene/Sky/SkyboxComponent.hpp>

#include "Engine.Assets.Type/Material/GfxMaterial.hpp"

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

	/**
	 *
	 */
	//_boundary = origin->getBoundaries();
	::hg::assertrt(origin->getSkyboxGeometryAsset().isValidType());
	_skyboxGeometryAsset = static_cast<ptr<assets::StaticGeometry>>(origin->getSkyboxGeometryAsset().internal());
	_skyboxGeometryResource = Engine::getEngine()->getResources()->loader().load(_skyboxGeometryAsset, nullptr);

	/**
	 *
	 */
	::hg::assertrt(origin->getSkyboxMaterialAsset().isValidType());
	auto materialAsset = static_cast<ptr<assets::GfxMaterial>>(origin->getSkyboxMaterialAsset().internal());
	auto materialResource = Engine::getEngine()->getResources()->loader().load(materialAsset, nullptr);

	_overrideMaterials.push_back(materialResource.into<MaterialResource>());
}

void SkyboxModel::update(const ptr<render::RenderSceneSystem> system_) {}

void SkyboxModel::destroy(const ptr<render::RenderSceneSystem> system_) {}

void SkyboxModel::capture(nmpt<render::MeshCaptureInterface> mci_) const noexcept {}

const ptr<engine::assets::StaticGeometry> SkyboxModel::geometryAsset() const noexcept {
	return _skyboxGeometryAsset;
}

cref<smr<engine::resource::ResourceBase>> SkyboxModel::geometryResource() const noexcept {
	return _skyboxGeometryResource;
}

bool SkyboxModel::streamable() const noexcept {
	return _streamable;
}
