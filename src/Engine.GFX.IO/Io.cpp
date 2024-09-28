#include "Io.hpp"

#include <Engine.Assets.Type/Geometry/StaticGeometry.hpp>
#include <Engine.Assets.Type/Texture/TextureAsset.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.Event/GlobalEventEmitter.hpp>
#include <Engine.GFX.Event/GraphicRegisterImporterEvent.hpp>
#include <Engine.GFX.Event/GraphicRegisterLoaderEvent.hpp>
#include <Engine.GFX.Event/GraphicUnregisterImporterEvent.hpp>
#include <Engine.GFX.Event/GraphicUnregisterLoaderEvent.hpp>
#include <Engine.GFX.Loader/Font/FontLoader.hpp>
#include <Engine.GFX.Loader/Geometry/StaticGeometryLoader.hpp>
#include <Engine.GFX.Loader/Geometry/StaticGeometryResource.hpp>
#include <Engine.GFX.Loader/Material/MaterialLoader.hpp>
#include <Engine.GFX.Loader/Material/MaterialPrototypeLoader.hpp>
#include <Engine.GFX.Loader/Material/MaterialPrototypeResource.hpp>
#include <Engine.GFX.Loader/Material/MaterialResource.hpp>
#include <Engine.GFX.Loader/Texture/TextureLoader.hpp>
#include <Engine.GFX.Loader/Texture/TextureResource.hpp>
#include <Engine.GFX/Importer/FontFileTypes.hpp>
#include <Engine.GFX/Importer/FontImporter.hpp>
#include <Engine.GFX/Importer/ImageFileTypes.hpp>
#include <Engine.GFX/Importer/ImageImporter.hpp>
#include <Engine.GFX/Importer/ModelFileTypes.hpp>
#include <Engine.GFX/Importer/ModelImporter.hpp>

using namespace ::hg::engine::gfx::loader;
using namespace ::hg;

template <typename LoaderType_, typename AssetType_, typename ResourceType_, typename... Args_>
static void registerIfNotPresent(ref<engine::resource::LoaderManager> manager_, Args_&&... args_) noexcept {

	if (manager_.hasLoader<AssetType_, ResourceType_>()) {
		return;
	}

	auto loader = ::hg::make_sptr<LoaderType_>(std::forward<Args_>(args_)...);
	manager_.registerLoader<AssetType_, ResourceType_>(std::move(loader));
}

template <typename ImporterType_, typename... Args_>
static void registerIfNotPresent(
	ref<engine::res::ImporterManager> manager_,
	cref<engine::res::FileTypeId> fileTypeId_,
	Args_&&... args_
) noexcept {

	if (manager_.hasImporter(fileTypeId_)) {
		return;
	}

	auto importer = ::hg::make_sptr<ImporterType_>(std::forward<Args_>(args_)...);
	manager_.registerImporter(fileTypeId_, importer);
}

/**/

void engine::gfx::loader::register_loader(
	ref<resource::LoaderManager> manager_,
	ref<Graphics> graphics_,
	ref<gfx::cache::GlobalCacheCtrl> cacheCtrl_,
	ref<gfx::pool::GlobalResourcePool> pool_
) {

	auto event = GraphicRegisterLoaderEvent { manager_, graphics_, cacheCtrl_, pool_ };
	Engine::getEngine()->getEmitter().emit(event);

	/**/

	registerIfNotPresent<TextureLoader, assets::TextureAsset, TextureResource>(
		manager_,
		std::addressof(cacheCtrl_),
		std::addressof(pool_),
		manager_.sharedSourceLoader()
	);

	registerIfNotPresent<StaticGeometryLoader, assets::StaticGeometry, StaticGeometryResource>(
		manager_,
		std::addressof(cacheCtrl_),
		std::addressof(pool_),
		manager_.sharedSourceLoader()
	);

	registerIfNotPresent<MaterialPrototypeLoader, assets::GfxMaterialPrototype, MaterialPrototypeResource>(
		manager_,
		std::addressof(cacheCtrl_),
		std::addressof(pool_),
		manager_.sharedSourceLoader()
	);

	registerIfNotPresent<MaterialLoader, assets::GfxMaterial, MaterialResource>(
		manager_,
		std::addressof(cacheCtrl_),
		std::addressof(pool_),
		manager_.sharedSourceLoader()
	);

	registerIfNotPresent<FontLoader, assets::Font, FontResource>(
		manager_,
		cacheCtrl_,
		pool_,
		manager_.sharedSourceLoader()
	);
}

void engine::gfx::loader::unregister_loader(
	ref<resource::LoaderManager> manager_,
	ref<Graphics> graphics_,
	ref<gfx::cache::GlobalCacheCtrl> cacheCtrl_,
	ref<gfx::pool::GlobalResourcePool> pool_
) {

	auto event = GraphicUnregisterLoaderEvent { manager_, graphics_, cacheCtrl_, pool_ };
	Engine::getEngine()->getEmitter().emit(event);

	/**/

	manager_.unregisterLoader<assets::TextureAsset>();
	manager_.unregisterLoader<assets::StaticGeometry>();
	manager_.unregisterLoader<assets::GfxMaterial>();
	manager_.unregisterLoader<assets::GfxMaterialPrototype>();
	manager_.unregisterLoader<assets::Font>();
}

void engine::gfx::loader::register_importer(ref<res::ImporterManager> manager_, ref<Graphics> graphics_) {

	auto event = GraphicRegisterImporterEvent { manager_, graphics_ };
	Engine::getEngine()->getEmitter().emit(event);

	/**/

	registerIfNotPresent<KtxImporter>(manager_, ImageFileType::Ktx);
	registerIfNotPresent<KtxImporter>(manager_, ImageFileType::Ktx2);
	registerIfNotPresent<FbxImporter>(manager_, ModelFileType::Fbx);
	registerIfNotPresent<FbxImporter>(manager_, ModelFileType::Obj);
	registerIfNotPresent<FontImporter>(manager_, FontFileType::Otf);
	registerIfNotPresent<FontImporter>(manager_, FontFileType::Ttf);
}

void engine::gfx::loader::unregister_importer(ref<res::ImporterManager> manager_, ref<Graphics> graphics_) {

	auto event = GraphicUnregisterImporterEvent { manager_, graphics_ };
	Engine::getEngine()->getEmitter().emit(event);

	/**/

	manager_.unregisterImporter(ImageFileType::Ktx);
	manager_.unregisterImporter(ImageFileType::Ktx2);
	manager_.unregisterImporter(ModelFileType::Fbx);
	manager_.unregisterImporter(ModelFileType::Obj);
	manager_.unregisterImporter(FontFileType::Otf);
	manager_.unregisterImporter(FontFileType::Ttf);
}
