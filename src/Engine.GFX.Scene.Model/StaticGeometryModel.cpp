#include "StaticGeometryModel.hpp"

#include <cstring>
#include <Engine.Assets.Type/Material/GfxMaterial.hpp>
#include <Engine.Common/Math/Convertion.hpp>
#include <Engine.Common/Math/Coordinates.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.GFX.Loader/Geometry/Traits.hpp>
#include <Engine.GFX.Loader/Material/Traits.hpp>
#include <Engine.GFX/Pool/SceneResourcePool.hpp>
#include <Engine.Reflect/Cast.hpp>
#include <Engine.Render.Scene/RenderSceneSystem.hpp>
#include <Engine.Resource/ResourceManager.hpp>
#include <Heliogrim/Component/Scene/Geometry/StaticGeometryComponent.hpp>

using namespace hg::engine::gfx::scene;
using namespace hg;

StaticGeometryModel::StaticGeometryModel(const ptr<SceneComponent> owner_) :
	InheritMeta(owner_),
	_sceneInstanceIndex(~0uL) {}

StaticGeometryModel::~StaticGeometryModel() {
	tidy();
}

void StaticGeometryModel::tidy() {}

void StaticGeometryModel::create(const ptr<render::RenderSceneSystem> system_) {

	auto origin = Cast<StaticGeometryComponent>(owner());
	const auto srp = system_->getSceneResourcePool();

	/**
	 *
	 */
	_staticGeometryAsset = static_cast<ptr<assets::StaticGeometry>>(origin->getStaticGeometryAsset().internal());
	_staticGeometryResource = Engine::getEngine()->getResources()->loader().
		load(_staticGeometryAsset, nullptr).
		into<StaticGeometryResource>();

	/**
	 *
	 */
	for (const auto& material : origin->overrideMaterials()) {
		auto* wrapped { static_cast<ptr<assets::GfxMaterial>>(material.internal()) };
		auto resource = Engine::getEngine()->getResources()->loader().load<assets::GfxMaterial, MaterialResource>(
			std::move(wrapped),
			{}
		);

		_overrideMaterials.push_back(resource);
	}

	/**/

	auto result = srp->staticInstancePool.acquire();
	_sceneInstanceIndex = result.instanceIndex;

	auto aabbStore = srp->staticAabbPool.acquire(_sceneInstanceIndex);

	/**/

	const auto transform = _owner->getUniverseMatrix();

	const auto page = result.dataView->pages().front();
	auto allocated = page->memory()->allocated();

	const auto innerOffset = result.dataView->offset() - page->resourceOffset();
	allocated->map(allocated->size);
	std::memcpy(static_cast<ptr<char>>(allocated->mapping) + innerOffset, &transform, sizeof(transform));
	allocated->unmap();
}

static bool isDirty = true;

void StaticGeometryModel::update(const ptr<render::RenderSceneSystem> system_) {

	auto origin = Cast<StaticGeometryComponent>(owner());
	const auto srp = system_->getSceneResourcePool();

	/* Geometry Changes */

	{
		const auto outer = origin->getStaticGeometryGuid();
		const auto inner = _staticGeometryAsset->get_guid();

		if (outer != inner) {

			_staticGeometryAsset = static_cast<ptr<assets::StaticGeometry>>(
				origin->getStaticGeometryAsset().internal()
			);
			_staticGeometryResource = Engine::getEngine()->getResources()->loader().load(
				_staticGeometryAsset,
				nullptr
			).into<StaticGeometryResource>();

		}
	}

	/* Material Changes */

	const auto count = origin->overrideMaterials().size();
	for (u32 matIdx = 0; matIdx < count; ++matIdx) {

		auto* outer = static_cast<ptr<assets::GfxMaterial>>(origin->overrideMaterials()[matIdx].internal());
		const auto& inner = _overrideMaterials[matIdx];

		if (inner->getAssociation() == nullptr) {
			continue;
		}

		// TODO: Check whether pointer comparison is actually safe, or whether should use guid compare
		if (inner->getAssociation() != outer) {

			_overrideMaterials[matIdx] = Engine::getEngine()->getResources()->loader().load<
				assets::GfxMaterial, MaterialResource
			>(
				std::move(outer),
				{}
			);

		}
	}

	/* Transform Changes */

	if (isDirty) {

		const auto transform = _owner->getUniverseMatrix();
		const auto dataView = srp->staticInstancePool.getDataView(_sceneInstanceIndex);

		const auto page = dataView->pages().front();
		auto allocated = page->memory()->allocated();

		const auto innerOffset = dataView->offset() - page->resourceOffset();
		allocated->map(allocated->size);
		std::memcpy(static_cast<ptr<char>>(allocated->mapping) + innerOffset, &transform, sizeof(transform));
		allocated->unmap();

		/**/

		auto aabbStore = srp->staticAabbPool.getDataView(_sceneInstanceIndex);
	}
}

void StaticGeometryModel::destroy(const ptr<render::RenderSceneSystem> system_) {

	auto origin = Cast<StaticGeometryComponent>(owner());
	const auto srp = system_->getSceneResourcePool();

	srp->staticAabbPool.release(_sceneInstanceIndex);
	srp->staticInstancePool.release(_sceneInstanceIndex);

}

void StaticGeometryModel::capture(nmpt<render::MeshCaptureInterface> mci_) const noexcept {

	const auto transform = _owner->getUniverseMatrix();

	/**/

	#if FALSE
    for (const auto& material : _overrideMaterials) {

        const auto guard = material->acquire(resource::ResourceUsageFlag::eRead);
        cmd_->bindMaterial(MaterialIdentifier {}, guard.imm());
    }

    const auto meshGuard = _staticGeometryResource->acquire(resource::ResourceUsageFlag::eRead);
    cmd_->bindStaticMesh(meshGuard.imm());

    cmd_->bindStaticMeshInstance(nullptr/*{ trans * rotation * scale }*/);
    cmd_->drawStaticMeshIdx(1uL, 0uL, meshGuard->indices()->size(), 0uL);
	#endif
}

const ptr<engine::assets::StaticGeometry> StaticGeometryModel::geometryAsset() const noexcept {
	return _staticGeometryAsset;
}

cref<smr<engine::gfx::StaticGeometryResource>> StaticGeometryModel::geometryResource() const noexcept {
	return _staticGeometryResource;
}
